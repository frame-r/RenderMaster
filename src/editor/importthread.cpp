#include "importthread.h"
#include <condition_variable>
#include <mutex>
#include <thread>
#include <queue>
#include <string>
#include <atomic>
#include "editorcore.h"
#include "resource_manager.h"
#include "core.h"
#include <QQueue>
#include <QString>
#include <QFileInfo>

using namespace std;

static int progreess_{0};
static std::mutex mutex_;
static std::condition_variable var;
static bool isexit;
static QQueue<QString> tasks;
static atomic<int> taskProgress_;
static atomic<int> fileProgress_;
static atomic<float> taskInv_;

void importProgressCallback(int fileProgress)
{
	fileProgress_ = fileProgress;
}

int ImportThread::GetImportProgress()
{
	int file = taskInv_ * fileProgress_;
	if (file)
		return taskProgress_ + file;
	return taskProgress_;
}

void ImportThread::AddTask(const QString& file)
{
	std::lock_guard<std::mutex> L{mutex_};

	// Append task
	tasks.enqueue(file);

	// Tell the worker
	var.notify_one();
}

void ImportThread::Init()
{
	while(1)
	{
		std::unique_lock<std::mutex> lck(mutex_);

		var.wait(lck, [] {return isexit || !tasks.empty(); });

		taskProgress_ = 0;
		size_t tasks_num = tasks.size();
		int tasksCompleted = 0;
		auto *core = editor->core;
		auto *resMan = core->GetResourceManager();

		while (!tasks.empty())
		{
			const auto& path = tasks.head();

			taskInv_ = 1.0f / tasks_num;
			taskProgress_ = int(100.0f * taskInv_ * tasksCompleted);

			QFileInfo info(path);

			editor->SetProgressBarMessage(info.fileName());

			resMan->Import(path.toLatin1().data(), importProgressCallback);
			tasksCompleted++;

			tasks.dequeue();
		}

		taskProgress_ = 100;
		fileProgress_ = 0;
		taskInv_ = 0.0f;
		editor->SetProgressBarMessage("");

		if (isexit)
			break;
	}
}

void ImportThread::Free()
{
	std::lock_guard<std::mutex> lck(mutex_);
	isexit = true;
	var.notify_one();
}
