#pragma once

namespace inexor {
namespace server {

    /// permission levels for server administration
    enum
    {
        PRIV_NONE = 0,
        PRIV_MASTER,
        PRIV_AUTH,
        PRIV_ADMIN
    };

};
};
