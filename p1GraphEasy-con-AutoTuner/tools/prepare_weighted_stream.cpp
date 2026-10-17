// Bounded-memory serialization of the same seed-2358 weights used by the first 200.
#include <algorithm>
#include <charconv>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

struct Output {
    std::ofstream f;std::string b;
    Output(const std::string &p):f(p,std::ios::binary){if(!f)throw std::runtime_error("open output "+p);b.reserve(8<<20);}
    void flush(){f.write(b.data(),b.size());if(!f)throw std::runtime_error("output write failed (disk full?)");b.clear();}
    void number(uint64_t x,char sep='\n'){char s[32];auto q=std::to_chars(s,s+32,x);b.append(s,q.ptr);b+=sep;if(b.size()>(8<<20))flush();}
    void floating(float x){b.append(reinterpret_cast<const char*>(&x),sizeof(x));if(b.size()>(8<<20))flush();}
    ~Output(){if(!b.empty())f.write(b.data(),b.size());}
};
uint64_t mix(uint64_t x){x+=UINT64_C(0x9e3779b97f4a7c15);x=(x^(x>>30))*UINT64_C(0xbf58476d1ce4e5b9);x=(x^(x>>27))*UINT64_C(0x94d049bb133111eb);return x^(x>>31);}
uint32_t weight(uint32_t u,uint32_t v,uint64_t salt){uint64_t k=(uint64_t(std::min(u,v))<<32)|std::max(u,v);return uint32_t(mix(k^salt)%255)+1;}
uint32_t parse(const char *&p,const char *end){while(p<end && (*p==' '||*p=='\t'||*p=='\r'))++p;uint32_t n;auto q=std::from_chars(p,end,n);if(q.ec!=std::errc())throw std::runtime_error("bad source vertex");p=q.ptr;return n;}
int main(int argc,char **argv){try{
    if(argc!=7)throw std::runtime_error("usage: BIN RAW OUTDIR SALT RAW_BOTH EXPECTED_N");
    std::string bin=argv[1],raw=argv[2],out=argv[3];uint64_t salt=std::stoull(argv[4]);bool both=std::stoi(argv[5]);
    int fd=open(bin.c_str(),O_RDONLY);if(fd<0)throw std::runtime_error("open binary");struct stat st;if(fstat(fd,&st))throw std::runtime_error("stat binary");
    void *map=mmap(nullptr,st.st_size,PROT_READ,MAP_SHARED,fd,0);if(map==MAP_FAILED)throw std::runtime_error("mmap binary");
    auto h=static_cast<const uint64_t*>(map);uint64_t n=h[2],m=h[3];if(n!=std::stoull(argv[6])||uint64_t(st.st_size)<64+8*n+4*m)throw std::runtime_error("binary header");
    auto ip=reinterpret_cast<const uint64_t*>(static_cast<const char*>(map)+64);auto dst=reinterpret_cast<const uint32_t*>(static_cast<const char*>(map)+64+8*n);
    uint64_t sum=0,xorhash=0;Output wf(out+"/graph.weights"),wel(out+"/graph.wel"),wadj(out+"/graph.wadj");
    wadj.b="WeightedAdjacencyGraph\n";wadj.number(n);wadj.number(m);
    for(uint64_t u=0;u<n;++u)wadj.number(ip[u]);
    for(uint64_t e=0;e<m;++e)wadj.number(dst[e]);
    for(uint64_t u=0;u<n;++u){uint64_t end=(u+1<n?ip[u+1]:m);for(uint64_t e=ip[u];e<end;++e){
        auto v=dst[e];if(v>=n)throw std::runtime_error("destination out of bounds");auto w=weight(u,v,salt);
        wf.floating(float(w));wel.number(u,' ');wel.number(v,' ');wel.number(w);wadj.number(w);
        uint64_t k=(u<<32)|v;auto fingerprint=mix(k^(uint64_t(w)*UINT64_C(0xd6e8feb86659fd93)));sum+=fingerprint;xorhash^=fingerprint;
    }}wf.flush();wel.flush();wadj.flush();
    std::ifstream input(raw);if(!input)throw std::runtime_error("open raw input");Output p1(out+"/input.original.wel");
    std::string line;uint64_t count=0,rawsum=0,rawxor=0;uint32_t maximum=0;
    while(std::getline(input,line)){
        auto begin=line.find_first_not_of(" \t\r");if(begin==std::string::npos || line[begin]=='#'||line[begin]=='%')continue;
        const char *p=line.data()+begin,*end=line.data()+line.size();auto u=parse(p,end),v=parse(p,end);if(u>=n||v>=n)throw std::runtime_error("source vertex out of bounds");
        auto w=weight(u,v,salt);p1.number(u,' ');p1.number(v,' ');p1.number(w);++count;maximum=std::max(maximum,std::max(u,v));
        auto add=[&](uint32_t a,uint32_t b){auto k=(uint64_t(a)<<32)|b;auto f=mix(k^(uint64_t(w)*UINT64_C(0xd6e8feb86659fd93)));rawsum+=f;rawxor^=f;};
        add(u,v);if(!both)add(v,u);
    }p1.flush();if(!input.eof())throw std::runtime_error("raw input read failed");
    if(count*(both?1:2)!=m || uint64_t(maximum)+1!=n || sum!=rawsum || xorhash!=rawxor)throw std::runtime_error("original direction/weighted edge fingerprints differ from prepared CSR");
    std::cout<<"WEIGHT_FORMATS_VERIFIED vertices="<<n<<" arcs="<<m<<" original_rows="<<count<<"\n";
    munmap(map,st.st_size);close(fd);return 0;
}catch(const std::exception&e){std::cerr<<e.what()<<"\n";return 1;}}
