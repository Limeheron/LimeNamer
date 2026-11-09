using System.Text;

namespace limenamer;

internal static class UTF8_INPUT
{
    private static readonly UTF8Encoding STRICT = new(false, true);

    internal static byte[]? try_encode(string text)
    {
        try {
            byte[] bytes = new byte[STRICT.GetByteCount(text) + 1];
            STRICT.GetBytes(text, 0, text.Length, bytes, 0);
            return bytes;
        } catch (EncoderFallbackException) {
            return null;
        }
    }

    internal static byte[] encode(string text)
    {
        return try_encode(text) ?? throw new ENGINE_EXCEPTION(RESULT.invalid_argument);
    }
}
