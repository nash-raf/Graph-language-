/* Only no-op launch costs are persisted. Body samples, TDG plans and thread
 * grants remain process-local. A matching entry is probed again before use. */
#include <fcntl.h>
#include <sched.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <stddef.h>

typedef struct {
    uint64_t magic, key, saved;
    double ns;
    uint64_t checksum;
} sgpl_launch_cache_record;
static pthread_once_t sgpl_launch_cache_once = PTHREAD_ONCE_INIT;
static uint64_t sgpl_launch_machine_key;
static char sgpl_launch_cache_directory[PATH_MAX];

static uint64_t sgpl_launch_hash(uint64_t h, const void *bytes, size_t count)
{
    const unsigned char *p = bytes;
    while (count--) { h ^= *p++; h *= UINT64_C(1099511628211); }
    return h;
}
static void sgpl_launch_cache_init(void)
{
    const char *enabled = getenv("SGPL_LAUNCH_CACHE");
    if (enabled && (!strcmp(enabled,"0") || !strcmp(enabled,"off"))) return;
    const char *directory = getenv("SGPL_LAUNCH_CACHE_DIR");
    int length;
    if (directory && *directory)
        length = snprintf(sgpl_launch_cache_directory, PATH_MAX, "%s", directory);
    else {
        const char *base = getenv("XDG_CACHE_HOME");
        if (base && *base)
            length = snprintf(sgpl_launch_cache_directory, PATH_MAX, "%s/sgpl-launch-v1", base);
        else {
            base = getenv("HOME");
            if (!base || !*base) return;
            char cache[PATH_MAX];
            length = snprintf(cache, sizeof(cache), "%s/.cache", base);
            if (length < 0 || length >= (int)sizeof(cache)) return;
            if (mkdir(cache,0700) && errno != EEXIST) return;
            length = snprintf(sgpl_launch_cache_directory, PATH_MAX, "%s/sgpl-launch-v1", cache);
        }
    }
    if (length < 0 || length >= PATH_MAX ||
        (mkdir(sgpl_launch_cache_directory,0700) && errno != EEXIST)) {
        sgpl_launch_cache_directory[0]=0;return;
    }
    uint64_t key = UINT64_C(1469598103934665603);
    /* Recompilation invalidates the cache, including a changed pool protocol. */
    const char build[] = "sgpl-epoch-launch-v1 " __DATE__ " " __TIME__ " " __VERSION__;
    key=sgpl_launch_hash(key,build,sizeof(build));
    struct utsname host={0};
    if (uname(&host)) {sgpl_launch_cache_directory[0]=0;return;}
    key=sgpl_launch_hash(key,&host,sizeof(host));
    FILE *cpu=fopen("/proc/cpuinfo","r");
    if (!cpu) {sgpl_launch_cache_directory[0]=0;return;}
    char line[4096];
    while (fgets(line,sizeof(line),cpu))
        if (!strncmp(line,"vendor_id",9) || !strncmp(line,"cpu family",10) ||
            !strncmp(line,"model",5) || !strncmp(line,"stepping",8) ||
            !strncmp(line,"microcode",9)) key=sgpl_launch_hash(key,line,strlen(line));
    fclose(cpu);
    const char *settings[]={"/sys/devices/system/cpu/online",
        "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor",
        "/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq",
        "/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq",
        "/sys/devices/system/cpu/intel_pstate/no_turbo"};
    for (size_t i=0;i<sizeof(settings)/sizeof(*settings);++i) {
        FILE *f=fopen(settings[i],"r");
        if(f) {while(fgets(line,sizeof(line),f)) key=sgpl_launch_hash(key,line,strlen(line));fclose(f);}
    }
    sgpl_launch_machine_key=key;
}
static int sgpl_launch_cache_path(const sgpl_loop_profile_desc *desc,int threads,
                                 int ephemeral,int total,char path[PATH_MAX],uint64_t *out)
{
    pthread_once(&sgpl_launch_cache_once,sgpl_launch_cache_init);
    if(!sgpl_launch_machine_key || !sgpl_launch_cache_directory[0])return 0;
    cpu_set_t affinity;CPU_ZERO(&affinity);
    if(sched_getaffinity(0,sizeof(affinity),&affinity))return 0;
    uint64_t h=sgpl_launch_hash(sgpl_launch_machine_key,&affinity,sizeof(affinity));
    int64_t shape[]={threads,ephemeral,total,sgpl_runtime_thread_count(),
        desc?desc->runtime_kind:SGPL_RUNTIME_PLAIN,desc?desc->mode:SGPL_LOOP_DOALL,
        desc?desc->env_size:0,desc?desc->num_priv_targets:0,desc?desc->doacross_num_sync_ids:0};
    h=sgpl_launch_hash(h,shape,sizeof(shape));
    if(desc && desc->num_priv_targets>0) {
        if(!desc->priv_offsets || desc->num_priv_targets>1048576)return 0;
        h=sgpl_launch_hash(h,desc->priv_offsets,(size_t)desc->num_priv_targets*sizeof(*desc->priv_offsets));
    }
    const char *names[]={"OMP_WAIT_POLICY","GOMP_SPINCOUNT","OMP_PROC_BIND","OMP_PLACES",
        "KMP_BLOCKTIME","SGPL_POOL_SPIN_US","SGPL_ALLOW_HT","SGPL_NUM_THREADS","OMP_NUM_THREADS"};
    for(size_t i=0;i<sizeof(names)/sizeof(*names);++i) {
        const char *v=getenv(names[i]);
        h=sgpl_launch_hash(h,names[i],strlen(names[i])+1);
        if(v)h=sgpl_launch_hash(h,v,strlen(v)+1);
    }
    *out=h;
    int n=snprintf(path,PATH_MAX,"%s/%016llx.bin",sgpl_launch_cache_directory,(unsigned long long)h);
    return n>0 && n<PATH_MAX;
}
static int sgpl_launch_cache_load(const sgpl_loop_profile_desc *desc,int threads,
                                 int ephemeral,int total,double *ns)
{
    char path[PATH_MAX];uint64_t key;
    if(!sgpl_launch_cache_path(desc,threads,ephemeral,total,path,&key))return 0;
    int fd=open(path,O_RDONLY|O_CLOEXEC|O_NOFOLLOW);if(fd<0)return 0;
    struct stat st;sgpl_launch_cache_record r;
    int ok=!fstat(fd,&st) && S_ISREG(st.st_mode) && st.st_uid==getuid() &&
        st.st_size==sizeof(r) && read(fd,&r,sizeof(r))==sizeof(r);
    close(fd);
    uint64_t now=(uint64_t)time(NULL);
    if(!ok || r.magic!=UINT64_C(0x5347504c4c415531) || r.key!=key ||
        r.saved>now || now-r.saved>3600 || !isfinite(r.ns) || r.ns<=0 || r.ns>1e9 ||
        r.checksum!=sgpl_launch_hash(UINT64_C(1469598103934665603),&r,offsetof(sgpl_launch_cache_record,checksum)))return 0;
    *ns=r.ns;return 1;
}
static void sgpl_launch_cache_store(const sgpl_loop_profile_desc *desc,int threads,
                                   int ephemeral,int total,double ns)
{
    if(!isfinite(ns) || ns<=0 || ns>1e9)return;
    char path[PATH_MAX],temporary[PATH_MAX];uint64_t key;
    if(!sgpl_launch_cache_path(desc,threads,ephemeral,total,path,&key))return;
    int n=snprintf(temporary,sizeof(temporary),"%s.tmpXXXXXX",path);
    if(n<0||n>=(int)sizeof(temporary))return;
    int fd=mkstemp(temporary);if(fd<0)return;
    sgpl_launch_cache_record r={.magic=UINT64_C(0x5347504c4c415531),.key=key,
        .saved=(uint64_t)time(NULL),.ns=ns};
    r.checksum=sgpl_launch_hash(UINT64_C(1469598103934665603),&r,offsetof(sgpl_launch_cache_record,checksum));
    int ok=write(fd,&r,sizeof(r))==sizeof(r);
    if(close(fd))ok=0;
    if(ok)ok=rename(temporary,path)==0;
    if(!ok)unlink(temporary);
}
