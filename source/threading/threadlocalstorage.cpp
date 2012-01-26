#include "bricks/threading/thread.h"
#include "bricks/threading/threadlocalstorage.h"

using namespace Bricks::Collections;

namespace Bricks { namespace Threading { namespace Internal {
	static Dictionary<ThreadID, AutoPointer<ThreadLocalStorageDictionary> > localStorage;

	static void ThreadLocalStorageDestructor()
	{
		localStorage.RemoveKey(GetCurrentThreadID());
	}

	ThreadLocalStorageDictionary& GetStorageDictionary()
	{
		return *localStorage.GetItem(GetCurrentThreadID(), autonew ThreadLocalStorageDictionary());
	}
} } }
