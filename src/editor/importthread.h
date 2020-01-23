#ifndef IMPORTTHREAD_H
#define IMPORTTHREAD_H
#include <QString>

namespace ImportThread
{
	void Init();
	void Free();

	// [0, 100]
	int GetImportProgress();

	void AddTask(const QString& file);

};

#endif // IMPORTTHREAD_H
