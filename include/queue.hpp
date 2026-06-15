#ifndef BH_QUEUE
#define BH_QUEUE

#include <cstdint>
#include "freertos/queue.h"
#include <bit>
#include <array>

namespace bh
{

    enum class Type
    {
        NORMAL = 0,
        ISR,
    };

    template <typename T, std::size_t S>
    class Queue
    {
    public:
        Queue() noexcept;
        ~Queue() noexcept;

        Queue(const Queue &) = delete;
        Queue &operator=(const Queue &) = delete;
        Queue(Queue &&) noexcept = delete;
        Queue &operator=(Queue &&) noexcept = delete;

        inline bool empty() const noexcept { return (uxQueueSpacesAvailable(m_queue) == S); };
        template <Type type = Type::NORMAL>
        bool pop(T &outItem) noexcept;
        template <Type type = Type::NORMAL>
        bool push(T item) noexcept;

    private:
        QueueHandle_t m_queue;
    };

    template <typename T, std::size_t S>
    Queue<T, S>::Queue() noexcept : m_queue(xQueueCreate(S, sizeof(T)))
    {
    }

    template <typename T, std::size_t S>
    Queue<T, S>::~Queue() noexcept
    {
        if (m_queue != nullptr)
        {
            vQueueDelete(m_queue);
        }
    }

    template <typename T, std::size_t S>
    template <Type type>
    bool Queue<T, S>::pop(T &outItem) noexcept
    {
        std::array<char, sizeof(T)> buffer{};
        BaseType_t success = pdFALSE;

        if constexpr (type == Type::NORMAL)
        {
            success = xQueueReceive(m_queue, buffer.data(), static_cast<TickType_t>(0));
        }
        else
        {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            success = xQueueReceiveFromISR(m_queue, buffer.data(), &xHigherPriorityTaskWoken);
            if (xHigherPriorityTaskWoken == pdTRUE)
            {
                portYIELD_FROM_ISR();
            }
        }

        if (success == pdTRUE)
        {
            outItem = std::bit_cast<T>(buffer);
            return true;
        }

        return false;
    }

    template <typename T, std::size_t S>
    template <Type type>
    bool Queue<T, S>::push(T item) noexcept
    {
        BaseType_t success = pdFALSE;

        if constexpr (type == Type::NORMAL)
        {
            success = xQueueSendToBack(m_queue, static_cast<const void *>(&item), static_cast<TickType_t>(0));
        }
        else
        {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            success = xQueueSendToBackFromISR(m_queue, static_cast<const void *>(&item), &xHigherPriorityTaskWoken);
            if (xHigherPriorityTaskWoken == pdTRUE)
            {
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
        }
        return (success == pdTRUE);
    }

}

#endif