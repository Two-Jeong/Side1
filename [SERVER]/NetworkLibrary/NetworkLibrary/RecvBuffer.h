#pragma once

class RecvBuffer
{
public:
    RecvBuffer();
    RecvBuffer(int buffer_size);
    ~RecvBuffer();

public:
    char* GetReadPos() { return &m_buffer[m_read_pos]; }
    char* GetWritePos() {return &m_buffer[m_write_pos]; }
    int GetRemainingSize() const { return m_max_buffer_size - m_write_pos; }
    int GetDataSize() const { return m_write_pos - m_read_pos; } 
    
public:
    bool OnRead(int data_size);
    bool OnWrite(int data_size);
    void Clean();
    
private:
    char* m_buffer;
    int m_max_buffer_size = 0;
    int m_read_pos = 0;
    int m_write_pos = 0;
};
