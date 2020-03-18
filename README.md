# Apoapse Protocol
The Apoapse Protocol is a C++ framework providing secure transfer and storage of data over the network using a message/command interface defined by a data-driven system.

[🚀 Find out more about Apoapse](https://apoapse.space/) | [📥 Receive all news and updates directly to your inbox](https://apoapse.space/newsletter-opt-in/)

## How to build
To compile the Apoapse Protocol as a static library, you can either use CMake or the provided Visual Studio solution using a C++ 17 compiler and STL.
Third party libraries are also used and must be placed in the ThirdParty directory before compiling. While some should be pulled automatically using git modules, others (the ones in bold) must be downloaded and placed manually in the ThirdParty folder:
* [**boost**](https://www.boost.org/)
* [**openssl**](https://www.openssl.org/)
* [**sqlite**](https://www.sqlite.org/)
* [Apoapse Very Simple Unit Test framework](https://github.com/apoapse/Apoapse-Very-Simple-Unit-Test-framework)
* [Magic Enum C++](https://github.com/Neargye/magic_enum)
* [cryptopp](https://github.com/weidai11/cryptopp)

## How to use
### Data structure system
All data on the Apoapse Protocol should be using the data structure system, which is a data-driven system that abstract database storage and low level network encoding into a same package. For instance the system allow to read data from the database, be edited on the server, sent to a client and automatically checked and saved on his own database.

This greatly simplifies the filling or reading of data, the transfer of data and the checks for the integrity of the same data. These data structures are also reusable and can be nested within each other.

### Example of data structure definition
```json
{
    "custom_types": [
		{
			"name": "sha256",
			"underlying_type": "blob",
			"min_length": 32,
			"max_length": 32
		}
	],

    "data_structures": [
        {
            "name": "client_login",
            "fields": [
                {
                    "name": "username",
                    "type": "text",
                    "required": true,
                    "uses":	{
                        "server_storage": false,
                        "client_storage": false,
                        "command": true
                    }
                },
                {
                    "name": "password",
                    "type": "sha256",
                    "required": true,
                    "uses":	{
                        "server_storage": false,
                        "client_storage": false,
                        "command": true
                    }
                }
            ]
        }
    ]
}
```

### Command System
There are two ways to transfer data on the Apoase Protocol: using a File Stream Connection that allows very large ammounts of data (gigabytes sized files for instance) to be send and the Commands which is the preferred way to send events and data through the network.

A command is abstracted as an event coming from a TCP connection, where a data structure can be attached and is defined using a data-driven system.

### Example of cmd definition
```json
{
	"commands":	[
        {
			"name": "login",
			"name_short": "cl",
			"datastructure": "client_login",
			"require_authentication": false,
			"only_non_authenticated": true,
			"reception": {
				"server": true,
				"client": false
			},
			"operation": {
				"register": false
			}
		}
    ]
}
```

### Implementation
This intialization code is shared between servers and clients implementations
```cpp
global = Global::CreateGlobal();
global->logger = std::make_unique<Logger>("log.txt");   // Log file
global->apoapseData = std::make_unique<ApoapseData>(GetDatastructureJson()); // The parameter is the json formated data structures definitions

// Acessing and intializing the database
boost::shared_ptr<IDatabase> databaseSharedPtr = LibraryLoader::LoadLibrary<IDatabase>("DatabaseImpl.sqlite");
global->database = databaseSharedPtr.get();
databaseSharedPtr->Initialize();

const char* dbParams[1];
dbParams[0] = "database.db";
if (databaseSharedPtr->Open(dbParams, 1))
{
    LOG << "Database accessed successfully.";

    DatabaseIntegrityPatcher dbIntegrity;
    if (!dbIntegrity.CheckAndResolve())
    {
        FatalError("The database integrity patcher has failed");
    }
}
else
{
    FatalError("Unable to access the database");
}

boost::asio::io_service mainIOService;

// TLS
auto tlsContext = ssl::context(ssl::context::sslv23);
tlsContext.set_options(ssl::context::default_workarounds | ssl::context::no_sslv2 | ssl::context::no_tlsv1_1 | ssl::context::no_tlsv1 | ssl::context::single_dh_use);
```

### Implement as a server
```cpp
global->isServer = true;
global->cmdManager = std::make_unique<ServerCmdManager>();  // ServerCmdManager is a class derivating from the CommandsManagerV2 class

// TLS certificates
tlsContext.set_default_verify_paths();
tlsContext.use_certificate_chain_file("certificate.crt");
tlsContext.use_private_key_file("certificate.key", boost::asio::ssl::context::pem);
tlsContext.use_tmp_dh_file("dh2048.pem");

ApoapseServer server{};
auto mainServer = std::make_unique<TCPServer>(mainIOService, 3000, TCPServer::Protocol::ip_v6);   // Boost Asio IO service, port and IP protocol
mainServer->StartAccept<ServerConnection>(this, std::ref(tlsContext));  // ServerConnection is a class derivating from the GenericConnection class

mainIOService.run();  // Run the Boost Asio IO service (blocking)
```

### Implement as a client
```cpp
global->isClient = true;
global->cmdManager = std::make_unique<ClientCmdManager>(*m_apoapseClient);  // ClientCmdManager is a class derivating from the CommandsManagerV2 class

auto connection = std::make_shared<ClientConnection>(*mainIOService, tlsContext, *this);    // ClientConnection is a class derivating from the GenericConnection class
connection->Connect("127.0.0.1", 3001); // Address, port

mainIOService.run();  // Run the Boost Asio IO service (blocking)
```