using System.Diagnostics;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;

namespace module_sample
{
    public class Program
    {
        public static void Main(string[] args)
        {
            Trace.Listeners.Add(new ConsoleTraceListener());
            IHost host = Host.CreateDefaultBuilder(args)
                .ConfigureServices(services =>
                {
                    services.AddHostedService<Module>();
                })
                .Build();

            host.Run();
        }
    }
}