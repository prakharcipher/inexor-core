#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/predef.h>

namespace inexor {
namespace filesystem {

std::string ExecutablePathWithoutBinary(const char *argv0);
std::string ExecutablePath(const char *argv0);
std::string ExecutablePathFallback(const char *argv0);

#if (BOOST_OS_CYGWIN || BOOST_OS_WINDOWS)

#include <Windows.h>

std::string ExecutablePath(const char *argv0)
{
    char buf[1024] = {0};
    DWORD ret = GetModuleFileNameA(NULL, buf, sizeof(buf));
    if (ret == 0 || ret == sizeof(buf))
    {
        return ExecutablePathFallback(argv0);
    }
    return buf;
}

#elif (BOOST_OS_MACOS)

#include <mach-o/dyld.h>

std::string ExecutablePath(const char *argv0)
{
    char buf[1024] = {0};
    uint32_t size = sizeof(buf);
    int ret = _NSGetExecutablePath(buf, &size);
    if (0 != ret)
    {
        return ExecutablePathFallback(argv0);
    }
    boost::system::error_code ec;
    boost::filesystem::path p(
        boost::filesystem::canonical(buf, boost::filesystem::current_path(), ec));
    return p.make_preferred().string();
}

#elif (BOOST_OS_SOLARIS)

#include <stdlib.h>

std::string ExecutablePath(const char *argv0)
{
    std::string ret = getexecname();
    if (ret.empty())
    {
        return ExecutablePathFallback(argv0);
    }
    boost::filesystem::path p(ret);
    if (!p.has_root_directory())
    {
        boost::system::error_code ec;
        p = boost::filesystem::canonical(
            p, boost::filesystem::current_path(), ec);
        ret = p.make_preferred().string();
    }
    return ret;
}

#elif (BOOST_OS_BSD)

#include <sys/sysctl.h>

std::string ExecutablePath(const char *argv0)
{
    int mib[4] = {0};
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PATHNAME;
    mib[3] = -1;
    char buf[1024] = {0};
    size_t size = sizeof(buf);
    sysctl(mib, 4, buf, &size, NULL, 0);
    if (size == 0 || size == sizeof(buf))
    {
        return ExecutablePathFallback(argv0);
    }
    std::string path(buf, size);
    boost::system::error_code ec;
    boost::filesystem::path p(
        boost::filesystem::canonical(
            path, boost::filesystem::current_path(), ec));
    return p.make_preferred().string();
}

#elif (BOOST_OS_LINUX)

#include <unistd.h>

std::string ExecutablePath(const char *argv0)
{
    char buf[1024] = {0};
    ssize_t size = readlink("/proc/self/exe", buf, sizeof(buf));
    if (size == 0 || size == sizeof(buf))
    {
        return ExecutablePathFallback(argv0);
    }
    std::string path(buf, size);
    boost::system::error_code ec;
    boost::filesystem::path p(
        boost::filesystem::canonical(
            path, boost::filesystem::current_path(), ec));
    return p.make_preferred().string();
}

#else

std::string ExecutablePath(const char *argv0)
{
    return ExecutablePathFallback(argv0);
}

#endif

std::string ExecutablePathWithoutBinary(const char *argv0)
{
    boost::filesystem::path p = ExecutablePath(argv0);
    return p.parent_path().make_preferred().string();
}

std::string ExecutablePathFallback(const char *argv0)
{
    if (argv0 == nullptr || argv0[0] == 0)
    {
        return "";
    }
    boost::system::error_code ec;
    boost::filesystem::path p(
        boost::filesystem::canonical(
            argv0, boost::filesystem::current_path(), ec));
    return p.make_preferred().string();
}

}
}

