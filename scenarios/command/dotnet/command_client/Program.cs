using command_client;

//Trace.Listeners.Add(new ConsoleTraceListener());

IHost host = Host.CreateDefaultBuilder(args)
    .ConfigureServices(services =>
    {
        services.AddHostedService<Worker>();
    })
    .Build();

host.Run();
