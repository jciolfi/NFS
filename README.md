# CS5600_practicum2
CS5600 Practicum 2 Spring 2023

By John Ciolfi and James Florez

# How to run
- Set the flash drive paths in `config/drive_path_settings.txt`. The paths are separated by a newline with no other extra whitespace.
- Set the server IP address and port number in `config/connection_settings.txt`. The port number comes first and is separated from the IP address by a newline with no other extra whitespace.
- Compile the code with `make` or `make all`.
- Run `./fserver` on one terminal/machine.
- To execute individual commands, run `./fget [action] [operand1] [operand2]`
  - action is one of (GET, INFO, MD, PUT, RM)
  - operand1 is the first operand for all actions.
  - operand2 is only required for actions that require a second operand.
- To execute preset test cases, run `./client_test.sh [sleep_time]` in another terminal/machine.
    - sleep_time is a nonnegative integer that is the time between commands.
- To test journaling replace one of the drive paths with a bad value or disconnect the usb drive and run `./client_test.sh [sleep_time]`. The journal `journal/server_journal.txt` will show the operations that need to be applied to the disconnected replica for consistency to be achieved. Then fix the drive path or reconnect the usb drive and run `./client_test.sh [sleep_time]` again. Before any new operations are applied the journal entries will be processed and applied to the reconnected drive to ensure consistency. Once this is done the journal will be empty.
- To test server-side code individually (not end-to-end), execute ./server-test.

# Assumptions
- The payload from the server always contains a status code, info message about the action, and data if applicable.
    - The status code follows a HTTP-like style with integers.
- The max length for a file path for a flash drive is 127 characters.
- The max size of a client request and server response is 8191 characters.

# Approach
- The request action (GET, INFO, MD, PUT, RM) is filtered and delegated to the corresponding handler. Any information is extracted (e.g. for PUT, the local file data is sent too, and for GET, the received file data is stored). The request is built and sent to the server, with each argument separated by a space.
- The server parses the request to find the action (GET, INFO, MD, PUT, RM) and also delegates the request to the corresponding handler. Write operations are applied to both drives, and if both fail, an error response will be generated. If one fails and one succeeds, a success response will be generated with a message explaining which drive succeeded. For read operations, the first drive is checked, and if it is up, then it is read from. If it is down, the second drive will be used if it is up.
- To achieve consistency between the two replica drives the journaling strategy is used. Journal entries are created only for write operations (PUT, MD, RM) when one of the drives performed the operation but the other did not. If both drives are not operational, the client is told to try again later. The index of the drive that was unable to perform the operation is written to a new line in `journal/server_journal.txt` followed by the original client request. Every time an operation is performed the journal is first read to see if it needs to be processed. If possible all the operations will be performed and the journal will be cleared. If the journal only contains operations for a single drive and that drive is still disconnected new operations are permitted and recorded. If the journal contains operations for both drives and they fail to be performed the client will not be allowed to make new requests because the drives are in an inconsistent state.


