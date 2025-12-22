#pragma once
#include <google/protobuf/message.h>

struct PacketHeader
{
    unsigned short packet_size;
    unsigned short protocol_no;
};

enum
{
    PACKET_SIZE_SIZEOF = sizeof(unsigned short),
    PACKET_PROTOCOL_SIZEOF = sizeof(unsigned short),
    PACKET_HEADER_SIZEOF = sizeof(struct PacketHeader),
};

class Packet
{

public:
    Packet();
    Packet(Packet* packet);
    ~Packet();

    void reserve_packet_buffer(int size);
    void set_packet(char* data, int size);
    void set_owner(class Session* session);
    Session* get_owner(); 
public:


    unsigned short get_size() const { return *m_buffer.data(); }
    unsigned short get_body_size() const { return (*m_buffer.data()) - PACKET_HEADER_SIZEOF; }
    unsigned short get_protocol() const { return *(m_buffer.data() + PACKET_SIZE_SIZEOF); }
    std::vector<char>& get_buffer() {return m_buffer; }
    
    void initialize(unsigned short protocol_number)
    {
        m_buffer.resize(PACKET_HEADER_SIZEOF);
        m_current_idx = PACKET_HEADER_SIZEOF;
        ::memcpy_s(get_protocol_ptr(), PACKET_PROTOCOL_SIZEOF, &protocol_number, PACKET_PROTOCOL_SIZEOF);
    }
    void finalize() { *(static_cast<unsigned short*>(get_size_ptr())) = static_cast<unsigned short>(m_current_idx); }

public:
    
    /* --------------------------------------------- push --------------------------------------------- */
    template<typename t>
    void push(t& data)
    {
        size_t data_size = sizeof(t);
        //TODO: stop(current_idx + data_size >= PACKET_MAX_SIZE)

        m_buffer.resize(m_buffer.size() + data_size);
        ::memcpy_s(get_current_idx_ptr(), data_size, &data, data_size);
        m_current_idx += static_cast<int>(data_size);
    }

    void push(void* data, int size)
    {
        //TODO: stop(current_idx + size >= PACKET_MAX_SIZE)
        
        m_buffer.resize(m_buffer.size() + size);
        ::memcpy_s(get_current_idx_ptr(), size, data, size);
        m_current_idx += size;
    }
    void push(char* data, int size) { push(reinterpret_cast<void*>(data), size); }

    void push(std::wstring& data)
    {
        //TODO: stop(current_idx + size >= PACKET_MAX_SIZE)
        size_t data_size = data.size() * sizeof(wchar_t);
        push(data_size);

        m_buffer.resize(m_buffer.size() + data_size);
        ::memcpy_s(get_current_idx_ptr(), data_size, data.c_str(), data_size);
        m_current_idx += static_cast<int>(data_size);
    }
    
    void push(google::protobuf::Message& message)
    {
        m_buffer.resize(m_buffer.size() + message.ByteSizeLong());
        message.SerializeToArray(get_current_idx_ptr(), message.ByteSizeLong());
        m_current_idx += message.ByteSizeLong();
    }
    
    template <typename... Types>
    void push(Types&... args)
    {
         (push(args), ...); // C++ 17 fold expression
    }
    
    /* --------------------------------------------- pop --------------------------------------------- */
    template<typename t>
    void pop(t& data)
    {
        size_t data_size = sizeof(t);
        
        ::memcpy_s(&data, data_size, get_current_idx_ptr(), data_size);
        m_current_idx += static_cast<int>(data_size);
    }
    
    void pop(void* data, size_t size)
    {
        //TODO: stop(current_idx + size >= PACKET_MAX_SIZE)
        
        ::memcpy_s(data, size, get_current_idx_ptr(), size);
        m_current_idx += static_cast<int>(size);
    }

    void pop(char* data, int size) { pop(reinterpret_cast<void*>(data), size); }

    void pop(std::wstring& data)
    {
        //TODO: stop(current_idx + size >= PACKET_MAX_SIZE)
        size_t data_size = 0;
        pop(data_size);
        data.resize(data_size / sizeof(wchar_t));
        
        ::memcpy_s(const_cast<wchar_t*>(data.data()), data_size, get_current_idx_ptr(), data_size);
        m_current_idx += static_cast<int>(data_size);
    }

    void pop(google::protobuf::Message& message, int message_size)
    {
        message.ParseFromArray(get_current_idx_ptr(), message_size);
        m_current_idx += static_cast<int>(message.ByteSizeLong());
    }
    
    void pop_message(google::protobuf::Message& message)
    { 
        message.ParseFromArray(m_buffer.data() + PACKET_HEADER_SIZEOF, get_size() - PACKET_HEADER_SIZEOF);
    }
    
    template <typename... Types>
    void pop(Types&... args)
    {
        (pop(args), ...); // C++ 17 fold expression
    }
    
private:
    void* get_protocol_ptr() { return m_buffer.data() + PACKET_SIZE_SIZEOF; }
    void* get_size_ptr() { return m_buffer.data(); }
    void* get_current_idx_ptr() {return (m_buffer.data() + m_current_idx); }

private:
    std::vector<char> m_buffer; // TODO: use buffer pool
    int m_current_idx;

    // For Read 
    Session* m_owner;
};

#define DEFINE_SERIALIZER(...) \
    virtual void Write(Packet& p) \
    { \
        p.push(__VA_ARGS__); \
    } \
    virtual void Read(Packet& p) \
    { \
        p.pop(__VA_ARGS__); \
    } \


#define DEFINE_SERIALIZER_WITH_PARENT(parent, ...) \
    virtual void Write(Packet& p) \
    { \
        parent::Write(p); \
        p.push(__VA_ARGS__); \
    } \
    virtual void Read(Packet& p) \
    { \
        parent::Read(p); \
        p.pop(__VA_ARGS__); \
    } \
