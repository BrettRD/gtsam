/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation,
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 *  @file HybridFactor.h
 *  @date Mar 11, 2022
 *  @author Fan Jiang
 */

#pragma once

#include <gtsam/base/Testable.h>
#include <gtsam/discrete/DiscreteKey.h>
#include <gtsam/inference/Factor.h>
#include <gtsam/nonlinear/Values.h>
#include <gtsam/linear/GaussianFactorGraph.h>
#include <gtsam/discrete/DecisionTree.h>

#include <cstddef>
#include <string>
namespace gtsam {

class HybridValues;

/// Gaussian factor graph and log of normalizing constant.
struct GraphAndConstant {
  GaussianFactorGraph graph;
  double constant;

  GraphAndConstant(const GaussianFactorGraph &graph, double constant)
      : graph(graph), constant(constant) {}

  // Check pointer equality.
  bool operator==(const GraphAndConstant &other) const {
    return graph == other.graph && constant == other.constant;
  }

  // Implement GTSAM-style print:
  void print(const std::string &s = "Graph: ",
             const KeyFormatter &formatter = DefaultKeyFormatter) const {
    graph.print(s, formatter);
    std::cout << "Constant: " << constant << std::endl;
  }

  // Implement GTSAM-style equals:
  bool equals(const GraphAndConstant &other, double tol = 1e-9) const {
    return graph.equals(other.graph, tol) &&
           fabs(constant - other.constant) < tol;
  }
};

/// Alias for DecisionTree of GaussianFactorGraphs
using GaussianFactorGraphTree = DecisionTree<Key, GraphAndConstant>;

KeyVector CollectKeys(const KeyVector &continuousKeys,
                      const DiscreteKeys &discreteKeys);
KeyVector CollectKeys(const KeyVector &keys1, const KeyVector &keys2);
DiscreteKeys CollectDiscreteKeys(const DiscreteKeys &key1,
                                 const DiscreteKeys &key2);

/**
 * Base class for *truly* hybrid probabilistic factors
 *
 * Examples:
 *  - MixtureFactor
 *  - GaussianMixtureFactor
 *  - GaussianMixture
 *
 * @ingroup hybrid
 */
class GTSAM_EXPORT HybridFactor : public Factor {
 private:
  bool isDiscrete_ = false;
  bool isContinuous_ = false;
  bool isHybrid_ = false;

 protected:
  // Set of DiscreteKeys for this factor.
  DiscreteKeys discreteKeys_;
  /// Record continuous keys for book-keeping
  KeyVector continuousKeys_;

 public:
  // typedefs needed to play nice with gtsam
  typedef HybridFactor This;  ///< This class
  typedef boost::shared_ptr<HybridFactor>
      shared_ptr;       ///< shared_ptr to this class
  typedef Factor Base;  ///< Our base class

  /// @name Standard Constructors
  /// @{

  /** Default constructor creates empty factor */
  HybridFactor() = default;

  /**
   * @brief Construct hybrid factor from continuous keys.
   *
   * @param keys Vector of continuous keys.
   */
  explicit HybridFactor(const KeyVector &keys);

  /**
   * @brief Construct hybrid factor from discrete keys.
   *
   * @param keys Vector of discrete keys.
   */
  explicit HybridFactor(const DiscreteKeys &discreteKeys);

  /**
   * @brief Construct a new Hybrid Factor object.
   *
   * @param continuousKeys Vector of keys for continuous variables.
   * @param discreteKeys Vector of keys for discrete variables.
   */
  HybridFactor(const KeyVector &continuousKeys,
               const DiscreteKeys &discreteKeys);

  /// Virtual destructor
  virtual ~HybridFactor() = default;

  /// @}
  /// @name Testable
  /// @{

  /// equals
  virtual bool equals(const HybridFactor &lf, double tol = 1e-9) const;

  /// print
  void print(
      const std::string &s = "HybridFactor\n",
      const KeyFormatter &formatter = DefaultKeyFormatter) const override;

  /// @}
  /// @name Standard Interface
  /// @{

  /**
   * @brief Compute the error of this Gaussian Mixture given the continuous
   * values and a discrete assignment.
   *
   * @param values Continuous values and discrete assignment.
   * @return double
   */
  virtual double error(const HybridValues &values) const = 0;

  /// True if this is a factor of discrete variables only.
  bool isDiscrete() const { return isDiscrete_; }

  /// True if this is a factor of continuous variables only.
  bool isContinuous() const { return isContinuous_; }

  /// True is this is a Discrete-Continuous factor.
  bool isHybrid() const { return isHybrid_; }

  /// Return the number of continuous variables in this factor.
  size_t nrContinuous() const { return continuousKeys_.size(); }

  /// Return the discrete keys for this factor.
  const DiscreteKeys &discreteKeys() const { return discreteKeys_; }

  /// Return only the continuous keys for this factor.
  const KeyVector &continuousKeys() const { return continuousKeys_; }

  /// @}

 private:
  /** Serialization function */
  friend class boost::serialization::access;
  template <class ARCHIVE>
  void serialize(ARCHIVE &ar, const unsigned int /*version*/) {
    ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(Base);
    ar &BOOST_SERIALIZATION_NVP(isDiscrete_);
    ar &BOOST_SERIALIZATION_NVP(isContinuous_);
    ar &BOOST_SERIALIZATION_NVP(isHybrid_);
    ar &BOOST_SERIALIZATION_NVP(discreteKeys_);
    ar &BOOST_SERIALIZATION_NVP(continuousKeys_);
  }
};
// HybridFactor

// traits
template <>
struct traits<HybridFactor> : public Testable<HybridFactor> {};

template <>
struct traits<GraphAndConstant> : public Testable<GraphAndConstant> {};

}  // namespace gtsam
