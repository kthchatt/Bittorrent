#include "transfer.h"

#define PORT 5703

int recievePiece(char *filePath, int pieceSize, int pieceIndex){
	byte[] buffer = new byte[pieceSize];
	Socket socket = new Socket(AddressFamily.InterNetwork,
	FILE *file = fopen(filePath, "wb");
	if(!file) return 0;

	fseek(file, pieceSize*pieceIndex, SEEK_SET);
	SocketType.Dgram, ProtocolType.Udp);

	socket.Bind(new IPEndPoint(IPAddress.Any, PORT));
	if(socket.Receive(buffer, 0, 1, SocketFlags.None) <= 0) return 0;

	fwrite(buffer, sizeof(buffer), 1, file);
	fclose(fwrite);

	return 1;
}
