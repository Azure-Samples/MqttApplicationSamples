using System.Diagnostics;

namespace MQTTnet.Client.Extensions;

public static class TaskTimeoutExtension
{
    public static async Task<T> TimeoutAfter<T>(this Task<T> source, TimeSpan timeout)
    {
        TimeSpan actualTimeout = timeout;
        if (Debugger.IsAttached)
        {
            actualTimeout = timeout.Add(TimeSpan.FromSeconds(300));
        }
        if (await Task.WhenAny(source, Task.Delay(actualTimeout)) != source)
        {
            throw new TimeoutException();
        }
        return await source;
    }
}