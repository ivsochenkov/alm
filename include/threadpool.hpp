/**
 *  author:   Yuriy Lobarev
 *  telegram: @forman
 *  phone:    +7(910)983-95-90
 *  email:    forman@anyks.com
 *  site:     https://anyks.com
 */

#ifndef __ANYKS_THREAD_POOL__
#define __ANYKS_THREAD_POOL__

/**
 * Стандартная библиотека
 */
#include <queue>
#include <mutex>
#include <vector>
#include <memory>
#include <thread>
#include <future>
#include <stdexcept>
#include <functional>
#include <condition_variable>


/**
 * anyks пространство имён
 */
namespace anyks {
	/**
	 * Класс пула потоков
	 */
	typedef class ThreadPool {
		private:
			// Сингнал остановки работы пула потоков
			bool volatile stop = false;
			// Количество потоков
			size_t threads = 1;
		private:
			// Тип очереди задач
			typedef std::queue <std::function <void()>> TaskQueue_t;
		private:
			// Очередь задач на исполнение
			TaskQueue_t tasks;
			// Мьютекс для разграничения доступа к очереди задач
			mutable std::mutex queue_mutex;
			// Рабочие потоки для обработки задач
			std::vector <std::thread> workers;
			// Условная переменная, контролирующая исполнение задачи
			std::condition_variable condition;
		private:
			/**
			 * work Метод обработки очереди задач в одном потоке
			 */
			void work() noexcept {
				// Запускаем бесконечный цикл
				for(;;){
					// Создаём текущее задание
					function <void()> task;
					// Ожидаем своей задачи в очереди потоков
					{
						// Выполняем блокировку уникальным мютексом
						std::unique_lock <std::mutex> lock(this->queue_mutex);
						// Если это не остановка приложения и список задач пустой, ожидаем добавления нового задания
						this->condition.wait(lock, [this]{return this->stop || !this->tasks.empty();});
						// Если это остановка приложения и список задач пустой, выходим
						if(this->stop && this->tasks.empty()) return;
						// Получаем текущее задание
						task = move(this->tasks.front());
						// Удаляем текущее задание
						this->tasks.pop();
					}
					// Задача появилась, исполняем ее и сообщаем о том, что задача выбрана из очереди
					task();
				}
			}
		public:
			/**
			 * wait Метод ожидания выполнения задач
			 */
			void wait() noexcept {
				{
					// Останавливаем работу потоков
					this->stop = true;
					// Создаем уникальный мютекс
					std::unique_lock <std::mutex> lock(this->queue_mutex);
				}
				// Создаем пустой список задач
				TaskQueue_t empty;
				// Сообщаем всем что мы завершаем работу
				this->condition.notify_all();
				// Ожидаем завершение работы каждого воркера
				for(std::thread & worker: this->workers) worker.join();
				// Очищаем список потоков
				this->workers.clear();
				// Очищаем список задач
				swap(this->tasks, empty);
				// Восстанавливаем работу потоков
				this->stop = false;
			}
			/**
			 * init Метод инициализации работы тредпула
			 * @param threads количество потоков
			 */
			void init(const size_t threads = 0) noexcept {
				// Ели количество потоков передано
				if(threads > 0) this->threads = threads;
				// Ели количество потоков передано
				if(this->threads > 0){
					// Добавляем в список воркеров, новую задачу
					for(size_t i = 0; i < this->threads; ++i){
						// Добавляем новую задачу
						this->workers.emplace_back(std::bind(&ThreadPool::work, this));
					}
				}
			}
			/**
			 * getTaskQueueSize Метод возврата количества сообщений в очереди задач на исполнение
			 * @return результат работы функции
			 */
			const size_t getTaskQueueSize() const noexcept {
				// Выполняем блокировку уникальным мютексом
				std::unique_lock <std::mutex> lock(this->queue_mutex);
				// Выводим количество заданий
				return this->tasks.size();
			}
			/**
			 * ThreadPool Конструктор
			 * @param threads потоки
			 */
			explicit ThreadPool(const size_t threads = std::thread::hardware_concurrency()) noexcept : stop(false), threads(0) {
				// Ели количество потоков передано
				if(threads > 0) this->threads = threads;
			}
			/**
			 * ~ThreadPool Деструктор
			 */
			~ThreadPool() noexcept {
				// Выполняем ожидание выполнения задач
				this->wait();
			}
		public:
			/**
			 * Шаблон метода добавления задач в пул
			 */
			template <class Func, class ... Args>
			/**
			 * push Метод добавления задач в пул
			 * @param func функция для обработки
			 * @param args аргументы для передачи в функцию
			 */
			auto push(Func && func, Args && ... args) noexcept -> std::future <typename std::result_of <Func(Args...)>::type> {
				// Устанавливаем тип возвращаемого значения
				using return_type = typename std::result_of <Func(Args...)>::type;
				// Добавляем задачу в очередь для последующего исполнения
				auto task = std::make_shared <std::packaged_task <return_type()>> (std::bind(std::forward <Func>(func), std::forward <Args> (args)...));
				// Создаем шаблон асинхронных операций
				std::future <return_type> res = task->get_future();
				{
					// Выполняем блокировку уникальным мютексом
					std::unique_lock <std::mutex> lock(this->queue_mutex);
					// Если это не остановка работы
					if(!this->stop) this->tasks.emplace([task](){(* task)();});
				}
				// Сообщаем потокам, что появилась новая задача
				this->condition.notify_one();
				// Выводим результат
				return res;
			}
	} tpool_t;
};

#endif // __ANYKS_THREAD_POOL__
