using leafdevice;
using Microsoft.ApplicationInsights.Extensibility.Implementation;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using System.Diagnostics;

using ConsoleTraceListener consoleListner = new();
Trace.Listeners.Add(consoleListner);
TelemetryDebugWriter.IsTracingDisabled = Debugger.IsAttached;

IHost host = Host.CreateDefaultBuilder(args)
    .ConfigureServices(services =>
    {
        services.AddApplicationInsightsTelemetryWorkerService();
        services.AddApplicationInsightsKubernetesEnricher();
        services.AddHostedService<Device>();
    })
    .Build();

await host.RunAsync();
