
#ifndef TEMPLATE_LINKED_LIST_HPP
#define TEMPLATE_LINKED_LIST_HPP

// Use only C++ 11 standard
// This class will never be inherited from

#include <atomic>
#include <mutex>

template <class T>
class tcDSLinkedList
{
    public:
        // Constructors and destructors
        tcDSLinkedList(void) { m_initialize(); }  // constructor
        ~tcDSLinkedList(void) { m_terminate(); }  // destructor

        // Disable copy constructor and assignment operators to prevent shallow copies
        tcDSLinkedList(const tcDSLinkedList&) = delete;
        tcDSLinkedList& operator=(const tcDSLinkedList&) = delete;

        T* head = nullptr;  /**< Pointer to the first node in the list (always exists) */
        T* tail = nullptr;  /**< Pointer to the last node in the list */

        std::uint32_t m_count = 0;
        std::mutex    m_listMutex;

        // Initialize
        void m_initialize(void)
        {
            std::lock_guard<std::mutex> lock(m_listMutex);
            // Initialize the head node
            head = new T; // Sentinel node
            head->next = nullptr;
            head->enabled = false;
            tail = head;
        }

        // Terminate
        void m_terminate(void)
        {
            std::lock_guard<std::mutex> lock(m_listMutex);
            // Delete all nodes in the list
            T* current = head;
            while (current != nullptr)
            {
                T* next = current->next;
                delete current;
                current = next;
            }
        }

        T* getNode(void)
        {
            std::lock_guard<std::mutex> lock(m_listMutex);

            T* current = head->next;

            // Look for an existing disabled node
            while (current != nullptr)
            {
                if (current->enabled == false)
                {
                    m_count++;
                    current->enabled = true;
                    return current;
                }
                current = current->next;
            }

            // No free node found, create a new one
            tail->next = new T{};
            tail = tail->next;
            tail->enabled = true;
            tail->ID = generateUniqueID();
            tail->next = nullptr;
            m_count++;
            return tail;
        }

        void releaseNode(T* node)
        {
            if (!node) return;

            std::lock_guard<std::mutex> lock(m_listMutex);

            node->enabled = false;
            node->data    = {};
            m_count--;

        }

        // swap node pointers
        void swapNodes(T* _node1, T* _node2)
        {
            if (!_node1 || !_node2 || _node1 == _node2)
                return;

            std::lock_guard<std::mutex> lock(m_listMutex);

            // Find previous nodes for both nodes
            T* prev1 = nullptr;
            T* prev2 = nullptr;
            T* current = head;

            while (current != nullptr && (prev1 == nullptr || prev2 == nullptr))
            {
                if (current->next == _node1)
                {
                    prev1 = current;
                }
                if (current->next == _node2)
                {
                    prev2 = current;
                }
                current = current->next;
            }

            // If either node not found, return
            if (prev1 == nullptr || prev2 == nullptr)
                return;

            // Update tail if necessary
            if (_node1 == tail)
                tail = _node2;

            else if (_node2 == tail)
                tail = _node1;

            // Swap the nodes
            if (prev1 == _node2)
            {
                // _node2 is immediately before _node1
                prev2->next = _node1;
                T* temp = _node1->next;
                _node1->next = _node2;
                _node2->next = temp;
            }
            else if (prev2 == _node1)
            {
                // _node1 is immediately before _node2
                prev1->next = _node2;
                T* temp = _node2->next;
                _node2->next = _node1;
                _node1->next = temp;
            }
            else
            {
                // Nodes are not adjacent
                T* temp = _node1->next;
                prev1->next = _node2;
                prev2->next = _node1;
                _node1->next = _node2->next;
                _node2->next = temp;
            }
        }

    private:
        std::uint32_t m_nextID = 1;

        std::uint32_t generateUniqueID(void)
        {
            return m_nextID++;
        }
};

#endif // TEMPLATE_LINKED_LIST_HPP

