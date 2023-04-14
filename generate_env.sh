scenario=$1
if [ "$scenario" = "telemetry" ]
then
    # generate all telemetry .env files
    bash generate_env.sh TELEMETRY_PRODUCER
    bash generate_env.sh TELEMETRY_PRODUCER_2
    SAMPLE=TELEMETRY_CONSUMER
elif [ "$scenario" == "getting_started" ]
then
    SAMPLE=GETTING_STARTED
elif [ "$scenario" == "command" ]
then
    # generate all command .env files
    bash generate_env.sh COMMAND_INVOKER
    SAMPLE=COMMAND_RECEIVER
else
    SAMPLE=$scenario
fi

source .env
destFile=${!ENV_FILE_VAR}
echo "### Options ###" > $destFile
echo "USE_TLS=\"${USE_TLS}\"" >> $destFile
echo "MQTT_VERSION=\"${MQTT_VERSION}\"" >> $destFile

echo -e "\n### Common connection settings ###" >> $destFile
echo "CA_FILE=\"${CA_FILE}\"" >> $destFile
echo "CA_PATH=\"${CA_PATH}\"" >> $destFile
echo "QOS=\"${QOS}\"" >> $destFile
echo "KEEP_ALIVE_IN_SECONDS=\"${KEEP_ALIVE_IN_SECONDS}\"" >> $destFile
echo "CLEAN_SESSION=\"${CLEAN_SESSION}\"" >> $destFile

echo -e "\n### Sample specific connection settings ###" >> $destFile
echo "BROKER_ADDRESS=\"${!BROKER_ADDRESS_VAR}\"" >> $destFile
echo "BROKER_PORT=\"${!BROKER_PORT_VAR}\"" >> $destFile
echo "CERT_FILE=\"${!CERT_FILE_VAR}\"" >> $destFile
echo "KEY_FILE=\"${!KEY_FILE_VAR}\"" >> $destFile
echo "CLIENT_ID=\"${!CLIENT_ID_VAR}\"" >> $destFile
echo "USERNAME=\"${!USERNAME_VAR}\"" >> $destFile
echo "PASSWORD=\"${!PASSWORD_VAR}\"" >> $destFile
