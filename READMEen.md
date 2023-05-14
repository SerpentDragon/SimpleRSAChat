# Client-server application based on the RSA encryption algorithm

This application includes client and server side programs that can exchange messages with each other. The exchange takes place with encryption based on the asynchronous RSA encryption algorithm

## Description
of the RSA algorithm RSA is an asymmetric cryptographic algorithm that uses two keys - public and private.

Encryption: The message is encrypted using the public key by exponentiation modulo.

Decryption: An encrypted message is decrypted using a private key by exponentiation modulo.

Choosing sufficiently large primes is an important step in key generation, since the cryptographic strength of the RSA algorithm is based on the complexity of factoring large integers.