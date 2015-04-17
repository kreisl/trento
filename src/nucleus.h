// TRENTO: Reduced Thickness Event-by-event Nuclear Topology
// Copyright 2015 Jonah E. Bernhard, J. Scott Moreland
// MIT License

#ifndef NUCLEUS_H
#define NUCLEUS_H

#include <memory>
#include <random>
#include <string>
#include <vector>

#include "fwd_decl.h"
#include "nucleon.h"

namespace trento {

// Alias for a smart pointer to a Nucleus.
using NucleusPtr = std::unique_ptr<Nucleus>;

/// Interface class to nucleus types.  A nucleus stores an ensemble of nucleons
/// and randomly samples their transverse positions.  It implements a standard
/// iterator interface through \c begin() and \c end() functions.  Iterating over
/// a \c Nucleus means iterating over its \c Nucleons.
class Nucleus {
 public:
  /// The canonical way to create a \c Nucleus.
  ///
  /// \param species standard symbol, e.g. "p" for proton or "Pb" for lead-208
  ///
  /// \return a smart pointer \c std::unique_ptr<Nucleus>
  ///
  /// \throw std::invalid_argument for unknown species
  ///
  /// \rst
  ///
  /// Example::
  ///
  ///   std::unique_ptr<Nucleus> lead_nucleus = Nucleus::create("Pb");
  ///   double radius = lead_nucleus->radius();
  ///   lead_nucleus->sample_nucleons(0);
  ///
  /// \endrst
  static NucleusPtr create(const std::string& species);

  /// Default virtual destructor for abstract base class.
  virtual ~Nucleus() = default;

  /// The "radius", i.e. the maximum distance at which a nucleon could be
  /// placed.
  virtual double radius() const = 0;

  /// Sample a new ensemble of nucleon positions.
  /// \param offset shift for each \em x position
  virtual void sample_nucleons(double offset) = 0;

  using iterator = std::vector<Nucleon>::iterator;
  using const_iterator = std::vector<Nucleon>::const_iterator;

  // non-const overload
  iterator begin() noexcept
  { return nucleons_.begin(); }
  iterator end() noexcept
  { return nucleons_.end(); }

  // const overload
  const_iterator begin() const noexcept
  { return nucleons_.begin(); }
  const_iterator end() const noexcept
  { return nucleons_.end(); }

  // forced const
  const_iterator cbegin() const noexcept
  { return nucleons_.cbegin(); }
  const_iterator cend() const noexcept
  { return nucleons_.cend(); }

 protected:
  /// Constructor only accessible by derived classes.
  /// \param A number of nucleons
  explicit Nucleus(std::size_t A);

  /// Set a \Nucleon position.  Perfect-forwards arguments to
  /// \c Nucleon::set_position.  Implemented this way because while \c Nucleus
  /// is a friend of \c Nucleon, its derived classes are not.  This grants
  /// limited access to the derived classes.
  template <typename... Args>
  void set_nucleon_position(Nucleon& nucleon, Args&&... args);

 private:
  /// Internal storage of Nucleon objects.
  std::vector<Nucleon> nucleons_;
};

/// A \c Proton is a trivial \c Nucleus with a single \c Nucleon.
class Proton : public Nucleus {
 public:
  /// Default constructor.
  Proton();

  /// Always zero.
  virtual double radius() const override;

  /// Will always place the nucleon at (x, y) = (offset, 0).
  virtual void sample_nucleons(double offset) override;
};

/// Samples nucleons from a spherically symmetric Woods-Saxon distribution.
/// For non-deformed heavy nuclei such as lead.
/// Woods-Saxon parameters from http://inspirehep.net/record/786828.
class WoodsSaxonNucleus : public Nucleus {
 public:
  /// \brief Create a nucleus with \em A nucleons and Woods-Saxon parameters
  /// \em R, \em a.
  WoodsSaxonNucleus(std::size_t A, double R, double a);

  /// The returned radius will actually be somewhat smaller than the true
  /// maximum radius, because the Woods-Saxon dist falls off very rapidly
  /// (exponentially).  Since the radius determines the impact parameter range,
  /// if it reported the true maximum, far too many events would have no
  /// participants.
  virtual double radius() const override;

  /// Sample uncorrelated Woods-Saxon nucleon positions.
  virtual void sample_nucleons(double offset) override;

 private:
  /// W-S parameters.
  const double R_, a_;

  /// W-S distribution object.  Since the dist does not have an analytic inverse
  /// CDF, approximate it as a piecewise linear dist.  For a large number of
  /// steps this is very accurate.
  mutable std::piecewise_linear_distribution<double> woods_saxon_dist_;
};

}  // namespace trento

#endif  // NUCLEUS_H
