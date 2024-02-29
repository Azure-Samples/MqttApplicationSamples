// Define the connection_settings module
pub mod connection_settings {
    // Import necessary modules
    use std::collections::HashMap;
    use std::env;
    use std::error::Error;

    // Define a struct to represent the connection settings
    #[derive(Debug)]
    pub struct ConnectionSettings {
        pub mqtt_host_name: String,
        pub mqtt_tcp_port: u16,
        pub mqtt_use_tls: bool,
        pub mqtt_clean_session: bool,
        pub mqtt_keep_alive_in_seconds: u16,
        pub mqtt_client_id: String,
        pub mqtt_username: Option<String>,
        pub mqtt_password: Option<String>,
        pub mqtt_ca_file: Option<String>,
        pub mqtt_cert_file: Option<String>,
        pub mqtt_key_file: Option<String>,
        pub mqtt_key_file_password: Option<String>,
    }

    // Implement a method to convert strings to integers
    fn convert_to_int(value: &str, name: &str) -> Result<u16, Box<dyn Error>> {
        value.parse::<u16>().map_err(|_| format!("{} must be an integer", name).into())
    }

    // Implement a method to convert strings to booleans
    fn convert_to_bool(value: &str, name: &str) -> Result<bool, Box<dyn Error>> {
        match value.to_lowercase().as_str() {
            "true" => Ok(true),
            "false" => Ok(false),
            _ => Err(format!("{} must be true or false", name).into()),
        }
    }

    // Define other helper functions or associated items here
    pub fn get_connection_settings(env_filename: Option<&str>) -> Result<ConnectionSettings, Box<dyn Error>> {
        // Load environment variables from .env file if provided
        if let Some(filename) = env_filename {
            dotenv::from_path(filename)?;
        }
    
        // Read environment variables into a HashMap
        let mut env_values: HashMap<String, String> = env::vars().collect();
    
        // Define default values
        let default_values: HashMap<&str, &str> = [
            ("MQTT_TCP_PORT", "8883"),
            ("MQTT_USE_TLS", "true"),
            ("MQTT_CLEAN_SESSION", "true"),
            ("MQTT_KEEP_ALIVE_IN_SECONDS", "30"),
        ]
        .iter()
        .cloned()
        .collect();
    
        // Merge default values with environment variables
        for (key, value) in default_values.iter() {
            env_values.entry(key.to_string()).or_insert(value.to_string());
        }
    
        // Extract connection settings from environment variables
        let mqtt_host_name = env_values.remove("MQTT_HOST_NAME").ok_or("MQTT_HOST_NAME must be set")?;
        let mqtt_tcp_port = convert_to_int(env_values.remove("MQTT_TCP_PORT").unwrap().as_str(), "MQTT_TCP_PORT")?;
        let mqtt_use_tls = convert_to_bool(env_values.remove("MQTT_USE_TLS").unwrap().as_str(), "MQTT_USE_TLS")?;
        let mqtt_clean_session = convert_to_bool(env_values.remove("MQTT_CLEAN_SESSION").unwrap().as_str(), "MQTT_CLEAN_SESSION")?;
        let mqtt_keep_alive_in_seconds = convert_to_int(env_values.remove("MQTT_KEEP_ALIVE_IN_SECONDS").unwrap().as_str(), "MQTT_KEEP_ALIVE_IN_SECONDS")?;
        let mqtt_client_id = env_values.remove("MQTT_CLIENT_ID").unwrap_or_else(|| "".to_string());
        let mqtt_username = env_values.remove("MQTT_USERNAME");
        let mqtt_password = env_values.remove("MQTT_PASSWORD");
        let mqtt_ca_file = env_values.remove("MQTT_CA_FILE");
        let mqtt_cert_file = env_values.remove("MQTT_CERT_FILE");
        let mqtt_key_file = env_values.remove("MQTT_KEY_FILE");
        let mqtt_key_file_password = env_values.remove("MQTT_KEY_FILE_PASSWORD");
    
        // Construct ConnectionSettings struct
        let settings = ConnectionSettings {
            mqtt_host_name,
            mqtt_tcp_port,
            mqtt_use_tls,
            mqtt_clean_session,
            mqtt_keep_alive_in_seconds,
            mqtt_client_id,
            mqtt_username,
            mqtt_password,
            mqtt_ca_file,
            mqtt_cert_file,
            mqtt_key_file,
            mqtt_key_file_password,
        };
    
        Ok(settings)
    }
}
