using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using STSModule;
using System.Diagnostics;

Trace.Listeners.Add(new ConsoleTraceListener());
IHost host = Host.CreateDefaultBuilder(args)
    .ConfigureServices(services => services.AddHostedService<ModuleBackgroundService>())
    .Build();

host.Run();