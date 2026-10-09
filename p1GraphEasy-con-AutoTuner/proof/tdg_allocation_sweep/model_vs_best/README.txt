Read-only CSV analysis; no benchmark or production sources changed.
Best means fastest measured forced allocation + order + parent-worker limit, not an analytic optimum.
Times: median of process medians. Gap (%) = 100 * (model time / best time - 1).
Width 1 is serial. Teams need not coexist; each ordinary task uses one parent.
PNG tables show modal native loop widths; CSV also stores all observed pairs and schedules with counts.
For varying choices, model time aggregates actual native decisions rather than only the modal allocation.
Configuration: backend:left_width:right_width:order_code:parent_limit.
Mixed order_code = 2 * sequence_index + backfill; sequence list is in mixed_manifest.json.
Backend 0 = ordinary-first backup; 1 = dynamic executor; 2 = FIFO (new revision only).
Both policy tables share the same measured oracle. Shared mixed forced rows are counted as one result.
