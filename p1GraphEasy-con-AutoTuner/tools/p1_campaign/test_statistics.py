"""Exercise the directions and multiplicity correction used for promotion."""
import unittest
from types import SimpleNamespace
import runner as R
import numpy as np
from scipy.stats import permutation_test

class StatisticsContract(unittest.TestCase):
 def test_holm_is_monotone_and_familywise(self):
  self.assertEqual(R.holm([.001,.03,.02]),[.003,.04,.04])
 def test_one_sided_pair_direction(self):
  diff=np.linspace(.001,.002,20)
  test=lambda d:float(permutation_test((d,),lambda x,axis=-1:np.mean(x,axis=axis),permutation_type='samples',alternative='greater',n_resamples=100000,batch=512,rng=np.random.default_rng(2358)).pvalue)
  self.assertLess(test(diff)*248,.05)
  self.assertGreater(test(-diff),.99)
 def test_identical_binary_pair_is_not_a_regression(self):
  p=permutation_test((np.zeros(20),),lambda x,axis=-1:np.mean(x,axis=axis),permutation_type='samples',alternative='greater',n_resamples=100000,batch=512,rng=np.random.default_rng(2358)).pvalue
  self.assertEqual(p,1.)
 def test_geomean_direction(self):
  self.assertAlmostEqual(R.gm([2.,.5]),1.)

if __name__=='__main__':unittest.main()
