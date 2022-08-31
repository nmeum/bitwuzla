#ifndef BZLA_SOLVER_FP_ROUNDING_MODE_H_INCLUDED
#define BZLA_SOLVER_FP_ROUNDING_MODE_H_INCLUDED

namespace bzla {
namespace fp {

enum class RoundingMode
{
  RNA = 0,  // roundNearestTiesToAway
  RNE,      // roundNearestTiesToEven
  RTN,      // roundTowardNegative
  RTP,      // roundTowardPositive
  RTZ,      // roundTowardZero
  NUM_RM,
};

}  // namespace fp
}  // namespace bzla

#endif