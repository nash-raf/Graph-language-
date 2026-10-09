#!/usr/bin/env python3
"""Table II of Milo et al. 2003 (arXiv:cond-mat/0302375), recreated from the
paper's own formulas, and checked against the values printed in the PDF (p. 5).

  alpha_erdos = n - g                     (gamma >= gamma_c = s+1)
  alpha_sf    = n - g + s - gamma + 1     (2 < gamma < s+1)
  alpha_cond  = n - g + s - 1             (gamma <= 2)
"""

# (label, n, g, s, paper_alpha_erdos, paper_alpha_sf, paper_alpha_cond, paper_gamma_c)
# alpha_sf is given as (constant, coefficient of gamma).
PAPER = [
    ("3-node id6",   3, 2, 2,  1, (4, -1),  2, 3),
    ("3-node id12",  3, 2, 1,  1, (1,  0),  1, 2),
    ("3-node id14",  3, 3, 2,  0, (3, -1),  1, 3),
    ("3-node id36",  3, 2, 1,  1, (1,  0),  1, 2),
    ("3-node id38",  3, 3, 2,  0, (3, -1),  1, 3),
    ("3-node id46",  3, 4, 2, -1, (2, -1),  0, 3),
    ("3-node id74",  3, 3, 1,  0, (0,  0),  0, 2),
    ("3-node id78",  3, 4, 2, -1, (2, -1),  0, 3),
    ("3-node id98",  3, 3, 1,  0, (0,  0),  0, 2),
    ("3-node id102", 3, 4, 2, -1, (2, -1),  0, 3),
    ("3-node id108", 3, 4, 2, -1, (2, -1),  0, 3),
    ("3-node id110", 3, 5, 2, -2, (1, -1), -1, 3),
    ("3-node id238", 3, 6, 2, -3, (0, -1), -2, 3),
    ("4-node id14",  4, 3, 3,  1, (5, -1),  3, 4),
    ("4-node id204", 4, 4, 2,  0, (3, -1),  1, 3),
    ("4-node id206", 4, 5, 3, -1, (3, -1),  1, 4),
    ("4-node id2190",4, 5, 3, -1, (3, -1),  1, 4),
]


def fmt(term):
    c, k = term
    if k == 0:
        return str(c)
    if c == 0:
        return ("-" if k == -1 else f"{k}") + "gamma"
    return f"{c}{'-' if k < 0 else '+'}gamma"


def main():
    ok = True
    hdr = f"{'subgraph':>14} {'n':>2} {'g':>2} {'s':>2} {'a_erdos':>7} {'a_sf':>9} {'a_cond':>6} {'g_c':>4}"
    print(hdr)
    print("-" * len(hdr))
    for label, n, g, s, pe, psf, pc, pgc in PAPER:
        a_erdos = n - g
        a_cond = n - g + s - 1
        gc = s + 1
        sf_formula = (n - g + s + 1, -1)          # n-g+s-gamma+1
        # the paper prints the degenerate boundary value when gamma_c <= 2
        # (where the scale-free window is empty): n-g+s-1 for s<=1.
        sf_expected = sf_formula if gc > 2 else (n - g + s - 1, 0)
        bad = []
        if a_erdos != pe: bad.append("erdos")
        if a_cond != pc: bad.append("cond")
        if gc != pgc: bad.append("gamma_c")
        if psf != sf_expected:
            bad.append(f"sf(printed {fmt(psf)} vs {fmt(sf_expected)})")
        ok &= not bad
        print(f"{label:>14} {n:>2} {g:>2} {s:>2} {a_erdos:>7} "
              f"{fmt(psf):>9} {a_cond:>6} {gc:>4}   {'OK' if not bad else 'MISMATCH ' + ';'.join(bad)}")
    print()
    print("Table II (formula side):", "ALL OK" if ok else "FAILED")


if __name__ == "__main__":
    main()
