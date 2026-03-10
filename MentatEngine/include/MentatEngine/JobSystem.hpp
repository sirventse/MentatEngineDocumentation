#ifndef __JOBSYSTEM_HPP__
#define __JOBSYSTEM_HPP__ 1

#include <future>
#include <queue>
#include <thread>
#include <vector>
#include <functional>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <iostream>
#include <type_traits>
#include <utility>
#include <stdexcept>

class JobSystem {
public:
    /**
    * @brief Constructs the job system based on the number of threads.
    * @param threadNumber Number of available threads.
    */
    JobSystem(unsigned int threadNumber);
    ~JobSystem();

    JobSystem(const JobSystem& right) = delete;
    JobSystem& operator=(const JobSystem& right) = delete;

    JobSystem(JobSystem&& other) noexcept {
        std::unique_lock<std::mutex> lock(other.queue_mutex_);
        tasks_queue_ = std::move(other.tasks_queue_);
        workers_ = std::move(other.workers_);
        isStop_ = other.isStop_;
    }

    JobSystem& operator=(JobSystem&& other) noexcept {
        if (this != &other) {
            std::scoped_lock lock(queue_mutex_, other.queue_mutex_);
            isStop_ = other.isStop_;
            tasks_queue_ = std::move(other.tasks_queue_);
            workers_ = std::move(other.workers_);
        }
        return *this;
    }

    /**
    * @brief The main loop for worker threads.
    *
    * This method runs on each thread in the pool. It waits for new tasks
    * in the queue using a condition variable and executes them as they
    * become available until the system is stopped.
    */
    void worker();

    /**
     * @brief Enqueues a simple task that does not return a value.
     *
     * Use this for "fire-and-forget" tasks where no return value or
     * synchronization via std::future is required.
     *
     * @param task A move-only function (callable) to be executed.
     */
    void normalEnqueue(std::move_only_function<void()> task);

    /**
     * @brief Enqueues a task for asynchronous execution in the thread pool.
     *
     * This method wraps a function and its arguments into a packaged task,
     * pushes it into the job queue, and notifies a worker thread.
     *
     * @tparam F The type of the function or callable object.
     * @tparam Args The types of the arguments to be passed to the function.
     *
     * @param f The function to be executed by a worker thread.
     * @param args The specific arguments to forward to the function.
     *
     * @return A std::future that will eventually hold the result of the function execution.
     *
     * @throw std::runtime_error If the job system has been stopped and cannot accept new tasks.
     *
     * @note This method is thread-safe as it uses an internal mutex to protect the task queue.
     */
    template<class F, class... Args>
    auto tEnqueue(F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>>
    {
        using RetType = std::invoke_result_t<F, Args...>;

        // packaged guarda funcion para ejecutar mas tarde
        // lo devuelve a un shared porque la tarea la vamos a mover a la cola
        // std::bind crea una funcion sin parametros a partir de F y Args
        auto taskPtr = std::make_shared<std::packaged_task<RetType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        // almacenamos el future para obtenerlo mas tarde
        std::future<RetType> result = taskPtr->get_future();

        // BLOQUE PROTEGIDO
        {
            std::unique_lock<std::mutex> lock(queue_mutex_); // asi hacemos que solo un hilo toque la cola a la vez
            if (isStop_) { // si el sistema esta parado -> excepcion
                throw std::runtime_error("JobSystem stopped");
            }

            tasks_queue_.push(
                std::move_only_function<void()>(
                    [taskPtr]() { (*taskPtr)(); } // metemos en la cola una funcion, que cuando se ejecute, llamara a packaged_task
                )
            );
        }

        condition_.notify_one(); // despierta otr worker y coge nueva task
        return result; // devolvemos el future al usuario para que lo utilice cuando quiera
    }

    std::queue<std::move_only_function<void()>> tasks_queue_; // cola de funciones
    std::condition_variable condition_; // condicion para que los hilos esperen hasta qeu haya task a ejecutar
    std::vector<std::thread> workers_; // hilos
    std::mutex queue_mutex_; /// controlador para el acceso a la cola
    bool isStop_;
};

#endif // JOBSYSTEM


