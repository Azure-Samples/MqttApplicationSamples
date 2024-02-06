kubectl create secret tls localhost-tls --cert localhost.crt --key localhost.key
kubectl create configmap client-ca --from-file=chain.pem