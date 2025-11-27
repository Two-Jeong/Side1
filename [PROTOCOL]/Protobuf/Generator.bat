protoc --cpp_out=./ Protocols.proto

XCOPY /Y *.pb.cc "../ProtocolLibrary/ProtocolLibrary"
XCOPY /Y *.pb.h "../ProtocolLibrary/ProtocolLibrary"