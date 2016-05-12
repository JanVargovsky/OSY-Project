#ifndef SEMAPHORE_H
#define MSEMAPHORE_H

class Semaphore
{
    public:
        Semaphore(int key, int value);
        ~Semaphore();
        
        bool Up();
        bool Down();
    private:
    	const int key;
    	int id;
    	bool initialized;
};

#endif // SEMAPHORE_H

