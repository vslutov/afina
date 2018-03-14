#include <afina/Executor.h>

namespace Afina {

void
Executor::_run_task(std::unique_lock<std::mutex> &lock)
{
    if (tasks.size()) {
        auto exec = tasks.front();
        tasks.pop_front();
        ++ _busy_workers;
        lock.unlock();
        exec();
        lock.lock();
        -- _busy_workers;
    }
}

void *
Executor::perform(void *data)
{
    Executor *self = reinterpret_cast<Executor *>(data);
    std::unique_lock<std::mutex> lock(self->mutex);
    self->_run_task(lock); // May be we started from Execute

    while (true) {
        auto have_task = self->empty_condition.wait_for(lock, self->_idle_time, [self] { return self->tasks.size(); });

        if (have_task) {
            self->_run_task(lock);
        } else {
            if (self->threads.size() > self->_low_watermark) {
                return nullptr;
            }

            if (self->state == State::kStopping) {
                self->threads.erase(pthread_self());
                if (!self->threads.size()) {
                    self->empty_condition.notify_all();
                }
                return nullptr;
            }
        }
    }
}

Executor::Executor(size_t low_watermark, size_t high_watermark, size_t max_queue_size, size_t idle_time) :
  _low_watermark(low_watermark),
  _high_watermark(high_watermark),
  _max_queue_size(max_queue_size),
  _idle_time(idle_time),
  state(State::kRun)
{
    std::unique_lock<std::mutex> lock(mutex);
    for (size_t i = 0; i < low_watermark; ++i) {
        _add_thread();
    }
}

void
Executor::_add_thread()
{
    pthread_t tid;
    if (pthread_create(&tid, NULL, perform, this)) {
        throw std::runtime_error("Could not create thread");
    }
    threads.insert(tid);
}

void Executor::Stop(bool await) {
    std::unique_lock<std::mutex> lock;
    if (threads.size()) {
        state = State::kStopping;
        if (await) {
            empty_condition.wait(lock, [this] { return !threads.size(); });
        }
        state = State::kStopped;
    } else {
        state = State::kStopped;
    }
}

}
