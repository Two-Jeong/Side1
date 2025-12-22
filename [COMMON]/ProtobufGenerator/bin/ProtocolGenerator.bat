protoc --cpp_out=. Protocols.proto
protoc --csharp_out=. Protocols.proto
py enum_mapper_generator.py "./" "./PacketNumberMapper.h" "./PacketNumberMapper.cs"

COPY /Y *.pb.h "../../../[SERVER]\NetworkLibrary\NetworkLibrary"
COPY /Y *.pb.cc "../../../[SERVER]\NetworkLibrary\NetworkLibrary"
COPY /Y "PacketNumberMapper.h" "../../../[SERVER]\NetworkLibrary\NetworkLibrary"

COPY /Y *.cs "../../../[SERVER]/CSharp_NetworkClient/CSharp_NetworkClient"
COPY /Y "PacketNumberMapper.cs" "../../../[SERVER]/CSharp_NetworkClient/CSharp_NetworkClient"

COPY /Y *.cs "../../../[CLIENT]/Assets/02.Scripts/Network/Protocol"
COPY /Y "PacketNumberMapper.cs" "../../../[CLIENT]/Assets/02.Scripts/Network/Protocol"


pause 1 