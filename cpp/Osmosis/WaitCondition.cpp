#include <mutex>
#include "Osmosis/WaitCondition.h"

WaitCondition::WaitCondition( unsigned sleepInterval ) :
	_sleepInterval( sleepInterval ),
	_stopped( false )
{}

void WaitCondition::stop()
{
	{
		std::unique_lock< std::mutex > lock( _stopLock );
		_stopped = true;
	}
	_stop.notify_one();
}

bool WaitCondition::sleep()
{
	std::unique_lock< std::mutex > lock( _stopLock );
	if ( _stopped )
		return false;
	if ( _sleepInterval )
		_stop.wait_for( lock, std::chrono::seconds( _sleepInterval ) );
	else
		_stop.wait( lock );
	return not _stopped;
}
