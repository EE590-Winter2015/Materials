% First, open up a listening socket
import java.io.*;
import java.net.*;

% Create our server socket, bind it to port 5040
serv_sock = ServerSocket(5040);
disp('Created server socket on port 5040...');

% Wait until we have a client to talk to
client_sock = serv_sock.accept();
disp('Client connected!');

% Doesn't this look similar to C#?
client_input = BufferedReader(InputStreamReader(client_sock.getInputStream));
client_output = PrintWriter(client_sock.getOutputStream, true);

% Read in data, then spit something back out
data = client_input.readLine();
disp('Received: ');
disp(data)

disp('Sending back "YOLO"');
client_output.write(sprintf('YOLO\n'));
client_output.flush();

client_input.close()
client_output.close()
client_sock.close()
serv_sock.close()
