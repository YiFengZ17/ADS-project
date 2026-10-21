"""Check that the summary rejects warmups and incomplete or inconsistent runs."""
import unittest
from analyze import SCENARIOS, SIZES, TREES, validate, summarize


class AnalysisTests(unittest.TestCase):
    def setUp(self):
        """Build a complete synthetic grid with a known ten-value median."""
        self.rows = [dict(scenario=s, n=n, tree=t, repeat=r,
                          seed=(0x9e3779b9 ^ n ^ ((r + 1) * 0x85ebca6b)) & 0xffffffff,
                          insert_seconds=r + 1, delete_seconds=1, total_seconds=r + 2)
                     for s in SCENARIOS for n in SIZES for t in TREES for r in range(10)]

    def test_even_median(self):
        """Use the middle two runs, not one run or a sum of phase medians."""
        summary = list(summarize(validate(self.rows)))
        self.assertEqual(len(summary), 315)
        self.assertEqual(summary[0][-3:], [5500, 1000, 10000])
        self.assertEqual(summary[2][-3:], [6500, 2000, 11000])

    def test_bad_runs(self):
        """Reject incomplete grids, repeated rows, warmups, and bad values."""
        for field, value in [('repeat', -1), ('repeat', 10), ('seed', 1),
                             ('total_seconds', 100), ('insert_seconds', float('nan'))]:
            rows = [dict(row) for row in self.rows]
            rows[0][field] = value
            with self.subTest(field=field, value=value), self.assertRaises(ValueError):
                validate(rows)

        for rows in [self.rows[:-1], self.rows + [self.rows[0]]]:
            with self.assertRaises(ValueError):
                validate(rows)

    def test_total_is_not_sum_of_medians(self):
        """Keep paired total times when the phase medians do not add up."""
        for row in self.rows:
            row['delete_seconds'] = 100 if row['repeat'] == 0 else 1
            row['total_seconds'] = row['insert_seconds'] + row['delete_seconds']
        summary = list(summarize(validate(self.rows)))
        self.assertEqual(summary[2][-3], 7500)
        self.assertNotEqual(summary[2][-3], summary[0][-3] + summary[1][-3])


if __name__ == '__main__':
    unittest.main()
