#pragma once

#include <boost/random/mersenne_twister.hpp>           // for mt19937
#include <boost/random/uniform_int_distribution.hpp>   // for uniform_int_di...
#include <boost/random/uniform_real_distribution.hpp>  // for uniform_real_d...
#include <boost/uuid/seed_rng.hpp>                     // for seed
#include <limits>                                      // for numeric_limits
#include <type_traits>                                 // for conditional

namespace inexor {
namespace util {

namespace random {
    /// The default random engine for inexor
    typedef boost::random::mt19937 rng_engine;

    /// Wrapper around any RNG Generator that seeds it
    /// automatically, by calling boost::uuids::detail::seed
    template<typename T>
    class auto_seeded : public T {
    public:
        /// The default constructor automatically generates
        /// secure seeds
        auto_seeded() {
            boost::uuids::detail::seed(*this);
        }
    };

    /// The default rng, seeded with a time stamp from the
    /// best clock availabl (std::chrono::high_resolution_clock)
    extern thread_local auto_seeded<rng_engine> generator;

    /// The default rng for the deterministic_rng functions;
    /// this will be reseeded every time they're called.
    extern thread_local rng_engine deterministic_generator;

    /// The type of the seed the deterministic_generator expects
    typedef rng_engine::result_type seed_t;

    /// It is like the boost uniform distribution, but while
    /// that uses an closed distribution [min; max], this
    /// class uses a half opened one [min; max).
    ///
    /// This means that the boost one can return 1000 if
    /// max=1000, but this one can.
    template<typename T>
    class uniform_open_range_int_distribution
        : public boost::random::uniform_int_distribution<T> {
    public:
        explicit uniform_open_range_int_distribution(
            T min = 0
          , T max = std::numeric_limits<T>::max)
          : boost::random::uniform_int_distribution<T>(
              min, max - 1) {}
    };

    /// A generic uniform random number distribution that
    /// works on both floating point and integers
    ///
    /// This works by checking whether T is a floating point
    /// type; if it is, this template resolves to
    /// std::uniform_real_distribution, otherwise
    /// std::uniform_int_distribution.
    template<typename T>
    using uniform_generic_open_range_distribution = typename
        std::conditional< std::is_floating_point<T>::value,
            boost::random::uniform_real_distribution<T>,
            uniform_open_range_int_distribution<T>
        >::type;

    /// Generate a random number between Rmin and Rmax
    ///
    /// Internally this allocates
    /// a uniform_generic_distribution<T> on the fly and
    /// uses `generator` to actually retrieve the random
    /// number.
    ///
    /// Since the default genera
    /// function is thread safe.
    ///
    /// TODO: We could generate the randomness in an extra
    /// theread and pool it.
    ///
    /// @tparam T The type of random number to generate
    /// @return A number in the range `[Rmin, Rmax[`:
    ///   `Rmin <= rnd(Rmin, Rmax) < Rmax`
    template<typename T>
    T rnd(const T Rmin, const T Rmax) {
        // TODO. This could be expensive
        uniform_generic_open_range_distribution<T> dist(Rmin, Rmax);
        return dist(generator);
    }

    /// Generate a random number between 0 and Rmax
    ///
    /// The same as rnd(Rmin, Rmax), but with Rmin=0
    template<typename T>
    T rnd(const T Rmax) {
        return rnd(T(0), Rmax);
    }

    /// Generate any random number
    ///
    /// This generates any possible random number of the
    /// given type.
    ///
    /// Otherwise this is like rnd(Rmin, Rmax)
    template<typename T>
    T rnd_raw() {
        return rnd(std::numeric_limits<T>::min(),
                   std::numeric_limits<T>::max());
    }


    /// Generate a deterministic random number between Rmin
    /// and Rmax with a specific seed.
    ///
    /// Since the underlying generator is thread local, this
    /// function is thread safe.
    ///
    /// For a given seed this will always return the same
    /// value.
    template<typename T>
    T deterministic_rnd(const seed_t seed,
          const T Rmin, const T Rmax) {
        // TODO: Use a lookup table/cache?
        uniform_generic_open_range_distribution<T> dist(Rmin, Rmax);
        deterministic_generator.seed(seed);
        return dist(deterministic_generator);
    }

    /// Generate a deterministic random number between 0
    /// and Rmax with a specific seed.
    ///
    /// The same as deterministic_rnd(seed, Rmin, Rmax),
    /// but with Rmin=0
    template<typename T>
    T deterministic_rnd(const seed_t seed, const T Rmax) {
        return deterministic_rnd(seed, T(0), Rmax);
    }
}

// Export to inexor::util
using random::rnd;
using random::rnd_raw;
using random::deterministic_rnd;

}
}


