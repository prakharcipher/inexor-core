#pragma once

namespace inexor {
namespace server {


    INEXOR_FUNCTION_ALIAS(rnd, inexor::util::rnd<int>);
    INEXOR_FUNCTION_ALIAS(rndscale, inexor::util::rnd<float>);
    INEXOR_FUNCTION_ALIAS(detrnd, inexor::util::deterministic_rnd<int>);

};
};
