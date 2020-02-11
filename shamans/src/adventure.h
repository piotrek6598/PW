#ifndef SRC_ADVENTURE_H_
#define SRC_ADVENTURE_H_

#include <algorithm>
#include <atomic>
#include <cmath>
#include <vector>

#include "../third_party/threadpool/threadpool.h"

#include "./types.h"
#include "./utils.h"

class Adventure {
 public:
  virtual ~Adventure() = default;

  virtual uint64_t packEggs(std::vector<Egg>& eggs, BottomlessBag& bag) = 0;

  virtual void arrangeSand(std::vector<GrainOfSand>& grains) = 0;

  virtual Crystal selectBestCrystal(std::vector<Crystal>& crystals) = 0;
};

class LonesomeAdventure : public Adventure {
 public:
  LonesomeAdventure() {}

  /** @brief packEggs - packing eggs into BottomlessBag
   * @param eggs[in]       - reference to eggs' vector;
   * @param bag[in, out]   - reference to bag.
   * @return Maximum possible weight of packed eggs.
   */
  virtual uint64_t packEggs(std::vector<Egg>& eggs, BottomlessBag& bag) {
    std::vector<std::vector<uint64_t>> vec;
    vec.resize(eggs.size());
    for (size_t i = 0; i < vec.size(); i++)
      vec[i].resize(bag.getCapacity() + 1);
    for (uint64_t i = eggs[0].getSize(); i <= bag.getCapacity(); i++)
      vec[0][i] = eggs[0].getWeight();
    for (uint64_t i = 1; i < eggs.size(); i++) {
      for (uint64_t j = 0; j <= bag.getCapacity(); j++) {
        if (j < eggs[i].getSize()) {
          if (j != 0) {
            vec[i][j] = std::max(vec[i - 1][j], vec[i][j - 1]);
          } else {
            vec[i][j] = vec[i - 1][j];
          }
        } else {
          if (j != 0) {
            vec[i][j] = std::max(vec[i][j - 1], vec[i - 1][j]);
          } else {
            vec[i][j] = vec[i - 1][j];
          }
          vec[i][j] = std::max(vec[i][j], vec[i - 1][j - eggs[i].getSize()] +
                                              eggs[i].getWeight());
        }
      }
    }
    return vec[eggs.size() - 1][bag.getCapacity()];
  }

  /** @brief arrangeSand - Arranges sand's grains from smallest to largerst.
   * @param grains[in, out]   - reference to grains' vector.
   */
  virtual void arrangeSand(std::vector<GrainOfSand>& grains) {
    mergeSort(grains, 0, grains.size() - 1);
  }

  /** @brief selectBestCrystal - finds crystal with largest shininess.
   * @param crystals[in]   - reference to crystals' vector.
   * @return Crystal with largest shininess.
   */
  virtual Crystal selectBestCrystal(std::vector<Crystal>& crystals) {
    if (crystals.size() == 0) throw std::runtime_error("No crystals");
    Crystal bestCrystal = crystals[0];
    for (size_t i = 1; i < crystals.size(); i++) {
      bestCrystal = std::max(bestCrystal, crystals[i]);
    }
    return bestCrystal;
  }

 private:
  /** @brief merge - merges two sorted sub arrays to increasing sequnce.
   * @param grains[in, out]   - reference to grainns' vector;
   * @param l                 - first index of first sub array;
   * @param m                 - last index of first sub array;
   * @param r                 - last index of second sub array.
   */
  void merge(std::vector<GrainOfSand>& grains, size_t l, size_t m, size_t r) {
    std::vector<GrainOfSand> left, right;
    for (size_t i = 0; i < m - l + 1; i++) left.push_back(grains[l + i]);
    for (size_t i = 0; i < r - m; i++) right.push_back(grains[m + i + 1]);
    size_t i = 0, j = 0;
    size_t pos = l;
    while (i < left.size() && j < right.size()) {
      if (left[i] < right[j]) {
        grains[pos] = left[i];
        i++;
      } else {
        grains[pos] = right[j];
        j++;
      }
      pos++;
    }
    while (i < left.size()) {
      grains[pos] = left[i];
      i++;
      pos++;
    }
    while (j < right.size()) {
      grains[pos] = right[j];
      j++;
      pos++;
    }
  }

  /** @brief mergeSort - sorts given grains' sub array.
   * @param grains[in, out]   - reference to grains' vector;
   * @param l                 - first index of sub array;
   * @param r                 - last index of sub array.
   */
  void mergeSort(std::vector<GrainOfSand>& grains, size_t l, size_t r) {
    if (l < r) {
      size_t m = (l + r) / 2;
      mergeSort(grains, l, m);
      mergeSort(grains, m + 1, r);
      merge(grains, l, m, r);
    }
  }
};

class TeamAdventure : public Adventure {
 public:
  explicit TeamAdventure(uint64_t numberOfShamansArg)
      : numberOfShamans(numberOfShamansArg),
        councilOfShamans(numberOfShamansArg) {}

  /** @brief packEggs - packing egss into BottomlessBag with extra workers.
   * @param eggs[in]       - reference to eggs' vector
   * @param bag[in, out]   - reference to bag.
   * @return Maximum possible weight of packed eggs.
   */
  uint64_t packEggs(std::vector<Egg>& eggs, BottomlessBag& bag) {
    std::vector<std::vector<uint64_t>> vec;
    std::vector<std::vector<std::promise<uint64_t>>> promises;
    std::vector<std::vector<std::future<uint64_t>>> futures;
    vec.resize(eggs.size());
    promises.resize(eggs.size());
    futures.resize(eggs.size());
    for (size_t i = 0; i < eggs.size(); i++) {
      vec[i].resize(bag.getCapacity() + 1);
      promises[i].resize(numberOfShamans);
      futures[i].resize(numberOfShamans);
      for (uint64_t j = 0; j < numberOfShamans; j++)
        futures[i][j] = promises[i][j].get_future();
    }
    uint64_t mod = (bag.getCapacity() + 1) % numberOfShamans;
    uint64_t work_size = (bag.getCapacity() + 1) / numberOfShamans;
    uint64_t first = 0;
    uint64_t last = work_size - 1;
    // Distributing the work to shamans.
    for (uint64_t i = 0; i < numberOfShamans; i++) {
      if (i < mod) last++;
      councilOfShamans.enqueue(knapsack, std::ref(promises), std::ref(futures),
                               std::ref(eggs), std::ref(vec), first, last, i);
      first = last + 1;
      last += work_size;
    }
    // Waiting for result.
    futures[eggs.size() - 1][numberOfShamans - 1].get();
    return vec[eggs.size() - 1][bag.getCapacity()];
  }

  /** @brief arrangeSand - Arranges sand's grains from smallest to largest.
   * @param grains[in, out]   - reference to grains' vector.
   */
  virtual void arrangeSand(std::vector<GrainOfSand>& grains) {
    std::vector<std::pair<uint64_t, uint64_t>> ranges;
    std::vector<std::promise<bool>> promises;
    std::vector<std::future<bool>> futures;
    uint64_t leafs = 1;
    uint64_t position = 1;
    ranges.push_back({0, 0});
    promises.push_back(std::promise<bool>());
    futures.push_back(std::future<bool>());
    ranges.push_back({0, grains.size() - 1});
    promises.push_back(std::promise<bool>());
    futures.push_back(promises[1].get_future());
    // Distributing the work to shamans.
    while (leafs < numberOfShamans &&
           ranges[position].second - ranges[position].first != 0) {
      uint64_t m = (ranges[position].first + ranges[position].second) / 2;
      ranges.push_back({ranges[position].first, m});
      promises.push_back(std::promise<bool>());
      futures.push_back(promises[2 * position].get_future());
      ranges.push_back({m + 1, ranges[position].second});
      promises.push_back(std::promise<bool>());
      futures.push_back(promises[2 * position + 1].get_future());
      leafs++;
      position++;
    }
    size_t k = ranges.size() - 1;
    position = k;
    for (size_t i = 0; i < leafs; i++) {
      councilOfShamans.enqueue(mergeSortWrapper, std::ref(grains),
                               ranges[k - i].first, ranges[k - i].second,
                               std::ref(promises[k - i]));
    }
    // Merging sub results when it's possible.
    for (int i = ranges.size() - 1; i > 0; i -= 2) {
      if (i == 1) break;
      futures[i].get();
      futures[i - 1].get();
      councilOfShamans.enqueue(merge, std::ref(grains), ranges[i - 1].first,
                               ranges[i - 1].second, ranges[i].second,
                               std::ref(promises[i / 2]));
    }
    // Waiting for result.
    futures[1].get();
  }

  /** @brief selectBestCrystal - finds crystal with largest shininess.
   * @param crystals[in]   - reference to crystals' vector.
   * @return Crystal with largest shininess.
   */
  virtual Crystal selectBestCrystal(std::vector<Crystal>& crystals) {
    if (crystals.size() == 0) throw std::runtime_error("No crystals");
    // Setting optimal number of shamans to avoid situation when each worker
    // get too few work and communication's costs are too high.
    uint64_t used_shamans = std::min(numberOfShamans, getSqrt(crystals.size()));
    if (numberOfShamans - used_shamans <= 32) used_shamans = numberOfShamans;
    uint64_t mod = crystals.size() % used_shamans;
    uint64_t work_size = crystals.size() / used_shamans;
    std::vector<std::promise<Crystal>> promises;
    std::vector<std::future<Crystal>> futures;
    promises.resize(numberOfShamans);
    for (uint64_t i = 0; i < used_shamans; i++)
      futures.push_back(promises[i].get_future());

    if (work_size == 0) {
      for (size_t i = 0; i < crystals.size(); i++)
        promises[i].set_value(crystals[i]);
    } else {
      uint64_t first = 0;
      uint64_t last = work_size - 1;
      // Distributing the work.
      for (uint64_t i = 0; i < used_shamans; i++) {
        if (i < mod) last++;
        councilOfShamans.enqueue(bestCrystalPartial, first, last,
                                 std::ref(promises[i]), std::ref(crystals));
        first = last + 1;
        last += work_size;
      }
    }
    Crystal bestCrystal = futures[0].get();
    for (uint64_t i = 1; i < std::min(used_shamans, crystals.size()); i++)
      bestCrystal = std::max(futures[i].get(), bestCrystal);
    return bestCrystal;
  }

 private:
  /** @brief bestCrystalPartial - Function to execute for each worker.
   * Finds best crystal in given sub array.
   * @param beg            - first index of subarray;
   * @param end            - last index of sub array;
   * @param promise        - promise to fullfill with result;
   * @param crystals[in]   - reference to crystals' vector.
   */
  static void bestCrystalPartial(uint64_t beg, uint64_t end,
                                 std::promise<Crystal>& promise,
                                 std::vector<Crystal>& crystals) {
    Crystal bestCrystal = crystals[beg];
    for (uint64_t i = beg + 1; i <= end; i++)
      bestCrystal = std::max(bestCrystal, crystals[i]);
    promise.set_value(bestCrystal);
  }

  uint64_t getSqrt(size_t s) {
    uint64_t ret = 0;
    while (ret * ret < s) ret++;
    return ret;
  }

  /** @brief mergeSortWrapper - Wrapper for parallel merge sort.
   * Execute sequence mergeSort and fullfill promise when work was finished.
   * @param grains[in, out]   - reference to grains' vector;
   * @param l                 - first index of grains' subarray;
   * @param r                 - last index of grains' subarray;
   * @param promise[in, out]  - reference to promise.
   */
  static void mergeSortWrapper(std::vector<GrainOfSand>& grains, size_t l,
                               size_t r, std::promise<bool>& promise) {
    mergeSort(grains, l, r);
    promise.set_value(true);
  }

  /** @brief mergeSort - Sequence mergeSort for given subarray.
   * @param grains[in, out]   - reference to grains' vector;
   * @param l                 - first index of subarray;
   * @param r                 - last index of subarray.
   */
  static void mergeSort(std::vector<GrainOfSand>& grains, size_t l, size_t r) {
    if (l < r) {
      size_t m = (l + r) / 2;
      mergeSort(grains, l, m);
      mergeSort(grains, m + 1, r);
      // Merging sorted sub arrays.
      std::vector<GrainOfSand> left, right;
      for (size_t i = 0; i < m - l + 1; i++) left.push_back(grains[l + i]);
      for (size_t i = 0; i < r - m; i++) right.push_back(grains[m + i + 1]);
      size_t i = 0, j = 0;
      size_t pos = l;
      while (i < left.size() && j < right.size()) {
        if (left[i] < right[j]) {
          grains[pos] = left[i];
          i++;
        } else {
          grains[pos] = right[j];
          j++;
        }
        pos++;
      }
      while (i < left.size()) {
        grains[pos] = left[i];
        i++;
        pos++;
      }
      while (j < right.size()) {
        grains[pos] = right[j];
        j++;
        pos++;
      }
    }
  }

  /** @brief merge - Merges two sorted sub arrays sequentially.
   * @param grains[in, out]   - reference to grains vector;
   * @param l                 - first index of first sub array;
   * @param m                 - last index of first sub array;
   * @param r                 - last index of second sub array;
   * @param promise[in, out]  - promise to fullfill when work finished.
   */
  static void merge(std::vector<GrainOfSand>& grains, size_t l, size_t m,
                    size_t r, std::promise<bool>& promise) {
    std::vector<GrainOfSand> left, right;
    for (size_t i = 0; i < m - l + 1; i++) left.push_back(grains[l + i]);
    for (size_t i = 0; i < r - m; i++) right.push_back(grains[m + i + 1]);
    size_t i = 0, j = 0;
    size_t pos = l;
    while (i < left.size() && j < right.size()) {
      if (left[i] < right[j]) {
        grains[pos] = left[i];
        i++;
      } else {
        grains[pos] = right[j];
        j++;
      }
      pos++;
    }
    while (i < left.size()) {
      grains[pos] = left[i];
      i++;
      pos++;
    }
    while (j < right.size()) {
      grains[pos] = right[j];
      j++;
      pos++;
    }
    promise.set_value(true);
  }

  /** @brief knapsack - Partially solves knapsack problem.
   * Columns represent maximum possible weight of eggs not exceed bag's
   * capacity. Rows represent used eggs. Each worker solves problem for
   * given columns. Worker starts fullfilling n-th row, when previous
   * worker finished (n-1)-th row and inform when it finished row.
   * @param promises[in, out]   - reference to promises' vector;
   * @param futures[in, out]    - reference to futures' vector;
   * @param eggs[in]            - reference to eggs' vector;
   * @param vec                 - reference to vector with partial results;
   * @param beg                 - index of first column;
   * @param end                 - index of last column;
   * @param num                 - number of worker.
   */
  static void knapsack(
      std::vector<std::vector<std::promise<uint64_t>>>& promises,
      std::vector<std::vector<std::future<uint64_t>>>& futures,
      std::vector<Egg>& eggs, std::vector<std::vector<uint64_t>>& vec,
      uint64_t beg, uint64_t end, uint64_t num) {
    if (num != 0) futures[0][num - 1].get();
    for (size_t i = std::max(beg, eggs[0].getSize()); i <= end; i++) {
      vec[0][i] = eggs[0].getWeight();
    }
    promises[0][num].set_value(1);
    for (size_t i = 1; i < eggs.size(); i++) {
      // Waiting until previous worker finish row.
      if (num != 0) futures[i][num - 1].get();
      for (uint64_t j = beg; j <= end; j++) {
        if (j < eggs[i].getSize()) {
          if (j != 0) {
            vec[i][j] = std::max(vec[i - 1][j], vec[i][j - 1]);
          } else {
            vec[i][j] = vec[i - 1][j];
          }
        } else {
          if (j != 0) {
            vec[i][j] = std::max(vec[i][j - 1], vec[i - 1][j]);
          } else {
            vec[i][j] = vec[i - 1][j];
          }
          vec[i][j] = std::max(vec[i][j], vec[i - 1][j - eggs[i].getSize()] +
                                              eggs[i].getWeight());
        }
      }
      // Notifying about finished row.
      promises[i][num].set_value(1);
    }
  }

  uint64_t numberOfShamans;
  ThreadPool councilOfShamans;
};

#endif  // SRC_ADVENTURE_H_
