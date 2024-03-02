#pragma once

#include "zeroerr/internal/config.h"

#include <cstdint>
#include <cstring>
#include <vector>

ZEROERR_SUPPRESS_COMMON_WARNINGS_PUSH

namespace zeroerr {

/**
 * An extremely fast random generator. Currently, this implements *RomuDuoJr*, developed by Mark
 * Overton. Source: http://www.romu-random.org/
 *
 * RomuDuoJr is extremely fast and provides reasonable good randomness. Not enough for large
 * jobs, but definitely good enough for a benchmarking framework.
 *
 *  * Estimated capacity: @f$ 2^{51} @f$ bytes
 *  * Register pressure: 4
 *  * State size: 128 bits
 *
 * This random generator is a drop-in replacement for the generators supplied by ``<random>``.
 * It is not cryptographically secure. It's intended purpose is to be very fast so that
 * benchmarks that make use of randomness are not distorted too much by the random generator.
 *
 * Rng also provides a few non-standard helpers, optimized for speed.
 */
class Rng final {
public:
    /**
     * @brief This RNG provides 64bit randomness.
     */
    using result_type = uint64_t;

    static constexpr uint64_t(min)();
    static constexpr uint64_t(max)();

    /**
     * As a safety precausion, we don't allow copying. Copying a PRNG would mean you would have
     * two random generators that produce the same sequence, which is generally not what one
     * wants. Instead create a new rng with the default constructor Rng(), which is
     * automatically seeded from `std::random_device`. If you really need a copy, use copy().
     */
    Rng(Rng const&) = delete;

    /**
     * Same as Rng(Rng const&), we don't allow assignment. If you need a new Rng create one with
     * the default constructor Rng().
     */
    Rng& operator=(Rng const&) = delete;

    // moving is ok
    Rng(Rng&&) noexcept            = default;
    Rng& operator=(Rng&&) noexcept = default;
    ~Rng() noexcept                = default;

    /**
     * @brief Creates a new Random generator with random seed.
     *
     * Instead of a default seed (as the random generators from the STD), this properly seeds
     * the random generator from `std::random_device`. It guarantees correct seeding. Note that
     * seeding can be relatively slow, depending on the source of randomness used. So it is best
     * to create a Rng once and use it for all your randomness purposes.
     */
    Rng();

    /*!
      Creates a new Rng that is seeded with a specific seed. Each Rng created from the same seed
      will produce the same randomness sequence. This can be useful for deterministic behavior.
      @verbatim embed:rst
      .. note::
         The random algorithm might change between nanobench releases. Whenever a faster and/or
      better random generator becomes available, I will switch the implementation.
      @endverbatim
      As per the Romu paper, this seeds the Rng with splitMix64 algorithm and performs 10
      initial rounds for further mixing up of the internal state.
      @param seed  The 64bit seed. All values are allowed, even 0.
     */
    explicit Rng(uint64_t seed) noexcept;
    Rng(uint64_t x, uint64_t y) noexcept;
    Rng(std::vector<uint64_t> const& data);

    /**
     * Creates a copy of the Rng, thus the copy provides exactly the same random sequence as the
     * original.
     */
    Rng copy() const noexcept;

    /**
     * @brief Produces a 64bit random value. This should be very fast, thus it is marked as
     * inline. In my benchmark, this is ~46 times faster than `std::default_random_engine` for
     * producing 64bit random values. It seems that the fastest std contender is
     * `std::mt19937_64`. Still, this RNG is 2-3 times as fast.
     *
     * @return uint64_t The next 64 bit random value.
     */
    inline uint64_t operator()() noexcept {
        auto x = mX;

        mX = UINT64_C(15241094284759029579) * mY;
        mY = rotl(mY - x, 27);

        return x;
    }

    // This is slightly biased. See

    /**
     * Generates a random number between 0 and range (excluding range).
     *
     * The algorithm only produces 32bit numbers, and is slightly biased. The effect is quite
     * small unless your range is close to the maximum value of an integer. It is possible to
     * correct the bias with rejection sampling (see
     * [here](https://lemire.me/blog/2016/06/30/fast-random-shuffling/), but this is most likely
     * irrelevant in practices for the purposes of this Rng.
     *
     * See Daniel Lemire's blog post [A fast alternative to the modulo
     * reduction](https://lemire.me/blog/2016/06/27/a-fast-alternative-to-the-modulo-reduction/)
     *
     * @param range Upper exclusive range. E.g a value of 3 will generate random numbers 0,
     * 1, 2.
     * @return uint32_t Generated random values in range [0, range(.
     */
    inline uint32_t bounded(uint32_t range) noexcept {
        uint64_t r32         = static_cast<uint32_t>(operator()());
        auto     multiresult = r32 * range;
        return static_cast<uint32_t>(multiresult >> 32U);
    }

    // random double in range [0, 1]
    // see http://prng.di.unimi.it/

    /**
     * Provides a random uniform double value between 0 and 1. This uses the method described in
     * [Generating uniform doubles in the unit interval](http://prng.di.unimi.it/), and is
     * extremely fast.
     *
     * @return double Uniformly distributed double value in range [0,1], excluding 1.
     */
    inline double uniform01() noexcept {
        auto i = (UINT64_C(0x3ff) << 52U) | (operator()() >> 12U);
        // can't use union in c++ here for type puning, it's undefined behavior.
        // std::memcpy is optimized anyways.
        double d;
        std::memcpy(&d, &i, sizeof(double));
        return d - 1.0;
    }

    /**
     * Shuffles all entries in the given container. Although this has a slight bias due to the
     * implementation of bounded(), this is preferable to `std::shuffle` because it is over 5
     * times faster. See Daniel Lemire's blog post [Fast random
     * shuffling](https://lemire.me/blog/2016/06/30/fast-random-shuffling/).
     *
     * @param container The whole container will be shuffled.
     */
    template <typename Container>
    void shuffle(Container& container) noexcept {
        auto size = static_cast<uint32_t>(container.size());
        for (auto i = size; i > 1U; --i) {
            using std::swap;
            auto p = bounded(i);  // number in [0, i)
            swap(container[i - 1], container[p]);
        }
    }

    /**
     * Extracts the full state of the generator, e.g. for serialization. For this RNG this is
     * just 2 values, but to stay API compatible with future implementations that potentially
     * use more state, we use a vector.
     *
     * @return Vector containing the full state:
     */
    std::vector<uint64_t> state() const;

private:
    static constexpr uint64_t rotl(uint64_t x, unsigned k) noexcept;

    uint64_t mX;
    uint64_t mY;
};

} // namespace zeroerr

ZEROERR_SUPPRESS_COMMON_WARNINGS_POP