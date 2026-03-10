/**
 *
 * @brief Job system implementation.
 * @author Sergi Sirvent, ESAT 2025-2026
 * @subject Engine Programming
 *
 */

#include "../include/MentatEngine/JobSystem.hpp"

JobSystem::JobSystem(unsigned int threadNumber) : isStop_ {false} // inicializamos el sistema a false hasta qeu haya trabajo
{
	for (size_t i = 0; i < threadNumber; i++) { // inicializamos tantos hilos como se ahay indicado por parametro
		workers_.emplace_back(&JobSystem::worker, this); // cada hilo ejecuta la funcion worker() que esta dentro de: this
	}
}

JobSystem::~JobSystem()
{
	isStop_ = true;
	condition_.notify_all();

	while (!workers_.empty()) {
		workers_.back().join(); // espera a que terminen los hilos
		workers_.pop_back(); // los va scando del vector
	}
}

void JobSystem::worker()
{
	while (true) {
		std::move_only_function<void()> task; 
		{
			std::unique_lock<std::mutex> lock(queue_mutex_); // limita el acceso a la cola
			condition_.wait(lock, [this] () {return !tasks_queue_.empty() || isStop_; }); // se queda esperando a que isstop o a que haya alguna task que hacer

			if (isStop_ && tasks_queue_.empty()) return; // si no hay tareas o esta parado, el hilo sale del sistema
			task = std::move(tasks_queue_.front()); // coge la tarea
			
			tasks_queue_.pop();  // elimina la tarea de la cola al cogerla
		} // release lock (mutex)
		task(); // se ejecuta task
	}
}

void JobSystem::normalEnqueue(std::move_only_function<void()> task)
{
	{
		std::unique_lock<std::mutex> lock(queue_mutex_);  // limita el acceso a la cola
		tasks_queue_.push(std::move(task)); // coge una tarea de la cola
	} 

	condition_.notify_one(); // llama a un worker y lo despierta para la tarea
}

