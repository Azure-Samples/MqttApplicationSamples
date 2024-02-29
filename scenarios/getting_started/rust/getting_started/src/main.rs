use conn::connection_settings;
use dotenv::dotenv;
use paho_mqtt as mqtt;
use paho_mqtt::SslOptionsBuilder;
use paho_mqtt::SslVersion;
use paho_mqtt::MQTT_VERSION_5;
use std::time::Duration;

fn main() {
    // Initialize the logger from the environment
    env_logger::init();

    dotenv().ok();
    let conn_settings = connection_settings::get_connection_settings(Some(".env")).unwrap();
    let mqtt_clean_session = conn_settings.mqtt_clean_session;
    if !mqtt_clean_session {
        println!("This sample does not support connecting with existing sessions");
        std::process::exit(1);
    }
    let port = conn_settings.mqtt_tcp_port;
    let hostname = conn_settings.mqtt_host_name;
    let keepalive = conn_settings.mqtt_keep_alive_in_seconds;
    let client_id = conn_settings.mqtt_client_id;
    let mut protocol = "tcp";

    // Set up connection builder
    let mut conn_opts_builder = mqtt::ConnectOptionsBuilder::with_mqtt_version(MQTT_VERSION_5);
    conn_opts_builder
        .keep_alive_interval(Duration::from_secs(keepalive.into()))
        .clean_start(mqtt_clean_session);

    if let Some(username) = &conn_settings.mqtt_username {
        conn_opts_builder.user_name(username);
    }

    if let Some(password) = &conn_settings.mqtt_password {
        conn_opts_builder.password(password);
    }

    if conn_settings.mqtt_use_tls {
        protocol = "mqtts";

        // Create an SSL options builder
        let mut ssl_opts_builder = SslOptionsBuilder::new();
        ssl_opts_builder.ssl_version(SslVersion::Tls_1_2);

        // Trust store (CA file)
        if let Some(ca_file_path) = conn_settings.mqtt_ca_file {
            // Handle the Result returned by trust_store
            match ssl_opts_builder.trust_store(ca_file_path) {
                Ok(_) => {
                    // Trust store loaded successfully, continue
                }
                Err(err) => {
                    eprintln!("Failed to load trust store: {:?}", err);
                    return;
                }
            }
        }

        // Certificate file
        if let Some(cert_file_path) = conn_settings.mqtt_cert_file {
            // ssl_opts_builder = ssl_opts_builder.key_store(cert_file_path);
            match ssl_opts_builder.key_store(cert_file_path) {
                Ok(_) => {
                    // Key store loaded successfully, continue
                }
                Err(err) => {
                    eprintln!("Failed to load key store: {:?}", err);
                    return;
                }
            }
        }

        if let Some(key_file_path) = conn_settings.mqtt_key_file {
            match ssl_opts_builder.private_key(key_file_path) {
                Ok(_) => {
                    // Key store loaded successfully, continue
                }
                Err(err) => {
                    eprintln!("Failed to load private key: {:?}", err);
                    return;
                }
            }
        }

        if let Some(key_password) = conn_settings.mqtt_key_file_password {
            ssl_opts_builder.private_key_password(key_password);
        } else {
            // Handle the case where key_password is None, if needed.
        }
        let ssl_opts = ssl_opts_builder.finalize();
        conn_opts_builder.ssl_options(ssl_opts);
    }

    let host = format!("{}://{}:{}", protocol, hostname, port);
    println!("Host is {}", host);

    let create_options = mqtt::CreateOptionsBuilder::new()
        .server_uri(host)
        .client_id(client_id)
        .finalize();

    // Set up MQTT client
    let client = mqtt::Client::new(create_options).unwrap_or_else(|err| {
        eprintln!("Error creating the client: {}", err);
        std::process::exit(1);
    });

    // Initialize the consumer before connecting
    let rx = client.start_consuming();

    let conn_opts = conn_opts_builder.finalize();

    // Connect to the broker
    if let Err(err) = client.connect(conn_opts) {
        eprintln!("Failed to connect to the broker: {}", err);
        std::process::exit(1);
    }
    println!("Connected to the broker");

    // Subscribe to a topic
    // sample/#
    if let Err(err) = client.subscribe("sample/#", mqtt::QOS_0) {
        eprintln!("Failed to subscribe to topics: {}", err);
        std::process::exit(1);
    }
    println!("Subscribed to topics");

    // Publish a message
    let msg = mqtt::Message::new("sample/topic1", "hello world!", mqtt::QOS_0);
    if let Err(err) = client.publish(msg) {
        eprintln!("Failed to publish message: {}", err);
        std::process::exit(1);
    }
    println!("Published message");

    // ^C handler will stop the consumer, breaking us out of the loop, below
    let ctrlc_cli = client.clone();
    ctrlc::set_handler(move || {
        ctrlc_cli.stop_consuming();
    })
    .expect("Error setting Ctrl-C handler");

    // Just loop on incoming messages.
    println!("\nWaiting for messages..");
    for msg in rx.iter() {
        if let Some(msg) = msg {
            println!("Message received is {}", msg);
        } else if client.is_connected() {
            break;
        }
    }

    // Disconnect from the broker
    if let Err(err) = client.disconnect(None) {
        eprintln!("Failed to disconnect from the broker: {}", err);
        std::process::exit(1);
    }
    println!("Disconnected from the broker");
}
