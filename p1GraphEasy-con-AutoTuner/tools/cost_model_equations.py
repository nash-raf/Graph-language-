"""Reference equations and integer budget policy for review, not runtime wiring.

Inputs are per-site, per-regime isolated measurements. No machine constants or
benchmark names enter the policy. Pool service curves exclude time queued behind
other callers; parent scheduling and non-loop task work are separate inputs.
"""
from dataclasses import dataclass
from itertools import product


def doall_ns(n, largest_lane, serial_c, parallel_c, launch_ns, bandwidth_floor_ns=0, *, parallel=True):
    """T1=N*c_serial; TP=L+max(m*c_parallel, traffic/bandwidth)."""
    if not parallel:
        return n * serial_c
    return launch_ns + max(largest_lane * parallel_c, bandwidth_floor_ns)


def doacross_bounds_ns(work_ns, dependency_span_ns, launch_ns, sync_work_ns=0):
    """Ideal work-conserving envelope, NOT an upper bound on this runtime.

    Caller supplies work_ns already divided by effective width and a dependency
    span INCLUDING the required handoffs. Blocking wait time is not added again.
    The cyclic implementation can incur additional stalls. Metadata needed to
    compute the span is absent from the production loop descriptor.
    """
    return (launch_ns + max(work_ns + sync_work_ns, dependency_span_ns),
            launch_ns + work_ns + sync_work_ns + dependency_span_ns)


@dataclass(frozen=True)
class Site:
    task: int
    service_ns: dict  # widths -> isolated time; width 1 is the serial original
    pooled: bool = True


def level_ns(sites, widths, parent_width, parent_launch_ns=0, residuals=()):
    """Resource bound; exact for two one-site tasks ignoring launch staggering.

    A parent worker executes width-1 sites itself (zero extra loop budget).
    Every pooled width>1 site occupies the ONE shared pool exclusively.
    Ephemeral sites have no pool exclusion. Serial sites can overlap the pool.
    Group sites belonging to a task before computing the task critical path.
    For larger task graphs this is a lower-bound estimate, not a schedule proof.
    """
    task_costs = dict(enumerate(residuals))
    pooled_service = 0.0
    for site, width in zip(sites, widths):
        cost = site.service_ns[width]
        task_costs[site.task] = task_costs.get(site.task, 0.0) + cost
        if width > 1 and site.pooled:
            pooled_service += cost
    if not task_costs:
        return parent_launch_ns
    return parent_launch_ns + max(pooled_service, max(task_costs.values()),
                                  sum(task_costs.values()) / parent_width)


def choose_budget(sites, total_budget, parent_width, parent_launch_ns=0, residuals=()):
    """Exact discrete reference search, including serial and idle allocations.

    Exhaustive search is for validation, not a proposed large-level runtime
    solver. A production solver should optimize the same resource objective
    using bounded integer search and whole-objective repairs.
    """
    extra_budget = total_budget - parent_width
    if parent_width < 1 or extra_budget < 0:
        raise ValueError("parent width exceeds budget")
    best = None
    for widths in product(*(sorted(site.service_ns) for site in sites)):
        consumed = sum(w if w > 1 else 0 for w in widths)
        if consumed > extra_budget:
            continue
        time = level_ns(sites, widths, parent_width, parent_launch_ns, residuals)
        key = (time, consumed, widths)  # deterministic tie; retain idle capacity
        if best is None or key < best:
            best = key
    if best is None:
        raise ValueError("no feasible allocation in the supplied curves")
    return {"widths": best[2], "predicted_ns": best[0],
            "idle_threads": extra_budget - best[1]}
