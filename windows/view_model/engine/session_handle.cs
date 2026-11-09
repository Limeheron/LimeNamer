using Microsoft.Win32.SafeHandles;

namespace limenamer;

internal sealed class SESSION_HANDLE : SafeHandleZeroOrMinusOneIsInvalid
{
    public SESSION_HANDLE()
        : base(true)
    {
    }

    protected override bool ReleaseHandle()
    {
        NATIVE_METHODS.ln_session_destroy(handle);
        return true;
    }
}
