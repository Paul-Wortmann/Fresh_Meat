// !!! This implementation is custom designed for this project. !!!

// !!!! Warning, take care when using this template, it does not manage the memory of the pointers used !!!!
//
// Example usage:
//
//    1. Define a struct with data and a next pointer
//
//    struct sEvent
//    {
//        sEvent* next = nullptr;
//        uint32  data = 0;
//    };
//
//    2. Create a cQueue with the struct defined above
//
//    cQueue<sEvent> m_event;
//
//    3. When pushing to the queue, allocate memory before pushing the pointer, do not free the memory
//
//    sEvent* event1 = new sEvent;
//    event1->data = 100;
//    m_event.push(event1);
//
//    4. When popping an event, use the data and free the pointer.
//
//    for (sEvent* tEvent = system.getEvent(); tEvent != nullptr; tEvent = system.getEvent())
//    {
//        std::cout << "Event data: " << tEvent->data << std::endl;
//        delete tEvent;
//    }
//

// Use only C++ 11 standard
// This class will never be inherited from

#ifndef TEMPLATE_QUEUE_HPP
#define TEMPLATE_QUEUE_HPP

#include <mutex>

template <typename T>
class tcQueue
{
    public:
        // Constructors and destructors
        tcQueue(void) { m_initialize(); }  // constructor
        ~tcQueue(void) { m_terminate(); }  // destructor
        tcQueue(const  tcQueue&) = delete; // copy constructor
        tcQueue& operator=(const tcQueue& _other) = delete;

        // Interface: get size
        std::uint32_t size(void)
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            return m_size;
        }

        // Interface: push
        void          push(T* _data)
        {
            if (!_data) return;  // Guard against null pointers
            std::lock_guard<std::mutex> lock(m_queueMutex);
            m_push(_data);
        }

        // Interface: pop
        T*            pop(void)
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            return m_pop();
        }

    private:
        // Data
        T*                   m_head = nullptr;
        T*                   m_tail = nullptr;
        std::uint32_t        m_size = 0;
        std::mutex m_queueMutex;  // Mutex for thread safety

        // Initialize
        void   m_initialize(void)
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            m_head = nullptr;
            m_tail = nullptr;
            m_size = 0;
        }

        // Terminate
        void   m_terminate(void)
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            while (m_head != nullptr)
            {
                T* temp = m_head;
                m_head = m_head->next;
                delete temp;
            }
            m_tail = nullptr;
            m_size = 0;
        }

        // Push (internal, assumes mutex is already locked)
        // The caller allocates memory for the passed pointer
        void   m_push(T* _data)
        {
            // Ensure the node's next pointer is null (clean state)
            _data->next = nullptr;

            // First element in queue
            if (m_tail == nullptr)
            {
                m_tail = _data;
            }
            // Add to end
            else
            {
                m_tail->next = _data;
                m_tail = m_tail->next;
            }
            if (m_head == nullptr)
            {
                m_head = m_tail;
            }
            m_size++;
        }

        // Pop (internal, assumes mutex is already locked)
        // The caller frees the memory of the returned pointer
        T*     m_pop(void)
        {
            if (m_head == nullptr)
            {
                return nullptr;  // Queue is empty
            }

            T* temp = m_head;
            m_head = m_head->next;

            if (m_head == nullptr)
            {
                m_tail = nullptr;  // Queue became empty
            }

            m_size--;

            // Detach the popped node from the queue
            temp->next = nullptr;

            return temp;
        }
};

#endif // TEMPLATE_QUEUE_HPP

