"""Check resource accounting and allocation choices, independent of timings."""
import unittest
from cost_model_equations import Site, choose_budget, level_ns


class ResourcePolicyTests(unittest.TestCase):
    def test_parallel_pool_sites_serialize(self):
        sites = [Site(0, {1: 100, 2: 40}), Site(1, {1: 200, 2: 70})]
        self.assertEqual(level_ns(sites, (2, 2), 2), 110)

    def test_inline_task_overlaps_pool_and_costs_no_extra_workers(self):
        sites = [Site(0, {1: 100, 2: 60}), Site(1, {1: 300, 2: 120})]
        self.assertEqual(level_ns(sites, (1, 2), 2), 120)
        result = choose_budget(sites, 4, 2)
        self.assertEqual(result["widths"], (1, 2))
        self.assertEqual(result["idle_threads"], 0)

    def test_idle_allocation_is_valid_and_best(self):
        sites = [Site(0, {1: 10, 2: 100}), Site(1, {1: 20, 2: 100})]
        result = choose_budget(sites, 6, 2)
        self.assertEqual(result["widths"], (1, 1))
        self.assertEqual(result["idle_threads"], 4)

    def test_serial_parent_and_sites_of_same_task_are_sequential(self):
        sites = [Site(0, {1: 10}), Site(0, {1: 20}), Site(1, {1: 15})]
        self.assertEqual(level_ns(sites, (1, 1, 1), 2), 30)
        self.assertEqual(level_ns(sites, (1, 1, 1), 1), 45)

    def test_ephemeral_sites_do_not_occupy_the_shared_pool(self):
        sites = [Site(0, {2: 40}, False), Site(1, {2: 70}, False)]
        self.assertEqual(level_ns(sites, (2, 2), 2), 70)

    def test_parent_budget_is_always_reserved(self):
        with self.assertRaises(ValueError):
            choose_budget([], 1, 2)


if __name__ == "__main__":
    unittest.main()
