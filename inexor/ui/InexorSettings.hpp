
#pragma once

#include <string>
#include <stdio.h>  /* defines FILENAME_MAX */

#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

#include "inexor/io/Logging.hpp"

#include "include/cef_app.h"

namespace inexor {
namespace ui {

class InexorSettings : public CefSettings
{
    public:

    InexorSettings(std::string executable_path)
    {
#ifdef CEF_USE_SANDBOX
        no_sandbox = false;
#else
        no_sandbox = true;
#endif
        ignore_certificate_errors = false;
        multi_threaded_message_loop = false;
        windowless_rendering_enabled = true;
        command_line_args_disabled = true;
        log_severity = LOGSEVERITY_INFO;
        single_process = false;
        remote_debugging_port = 9222;

        char base_path[FILENAME_MAX];
        GetCurrentDir(base_path, sizeof(base_path));
        CefString(&resources_dir_path).FromString(executable_path);
        CefString(&locales_dir_path).FromString(executable_path + "/locales");
        CefString(&log_file).FromString(std::string(base_path) + "inexorcef.log"); // TODO: make adaptable.
        Log.ui->info("init: cef: setting subprocess executable_path: {}", executable_path);
        CefString(&browser_subprocess_path).FromString(executable_path + "/cef_subprocess.exe");
    }

};

}
}


