# Crypto Hedge Fund

## Overview

The CHF project is a comprehensive solution for managing a crypto hedge fund. It is built on a Python server with integrated C++ blocks, operating seamlessly as a monolithic program. This multithreaded system incorporates object-oriented programming (OOP) principles, utilizes an SQLite database, and employs robust security measures such as password hashing and salting. Encryption is implemented using AES 256 CBC with a 16-bit IV for API keys.

## Features

### 1. User Authentication and Security

- Passwords are hashed and salted for enhanced security.
- API keys are encrypted using AES 256 CBC to ensure secure communication.

### 2. Binance Integration

- Connects to the Binance API using API keys to track trader transactions within the market.
- Facilitates hedge fund transactions seamlessly.

### 3. Transaction Recording

- Maintains a comprehensive record of all transactions conducted within the hedge fund.

### 4. Admin Panel

- Features an admin panel for efficient management and oversight.

### 5. Fund Structure

- Implements a well-thought-out fund structure to ensure equitable distribution of funds.

### 6. Client-Trader Relationship

- Allows clients to assign multiple traders and assess their performance based on portfolio growth percentage.
- Ensures trader work privacy by not exposing detailed portfolio information.

### 7. API Architecture

- Operates primarily as an API, allowing the development of various interfaces such as web apps and desktop apps.
- Accepts HTTP requests with JSON payloads.

## Additional Considerations

1. **Data Privacy:**
   - Portfolio details are not exposed to ensure trader privacy.

2. **API Usage:**
   - The system is designed as a flexible API, facilitating integration with various front-end applications.

3. **Client-Trader Relationship:**
   - Clients can assign multiple traders, ensuring a diverse and tailored investment strategy.

4. **Security Measures:**
   - Robust security measures, including encryption and hashing, are implemented to safeguard user data.
