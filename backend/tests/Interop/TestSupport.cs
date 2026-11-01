internal static class TestSupport
{
    public static void Require(bool condition, string message)
    {
        if (!condition) throw new Exception(message);
    }
    public static void Throws<T>(Action action) where T : Exception
    {
        try { action(); } catch (T) { return; }
        throw new Exception($"Expected {typeof(T).Name}.");
    }
}
