#ifndef EVENTBUS_H
#define EVENTBUS_H

#include "../Logger/Logger.h"
#include "Event.h"	
#include <map>
#include <typeindex>
#include <memory>
#include <list>

//ev = event
//owner = owner of instance




class IEventCallBack
{
	private:
		virtual void Call(Event& ev) = 0;

	public:
		virtual ~IEventCallBack() = default;
		void Exectue(Event& ev)
		{
			Call(ev);
		}
};



template <typename TOwner, typename TEvent>
class EventCallBack : public IEventCallBack
{
	private:
		typedef void(TOwner::*CallBackFun)(TEvent&);

		TOwner* owner;
		CallBackFun callbackFun;

		virtual void Call(Event& ev) override
		{
			std::invoke(callbackFun, owner, static_cast<TEvent&>(ev));
		}

	public:
		EventCallBack(TOwner* owner, CallBackFun callbackFun)
		{
			this->owner = owner;
			this->callbackFun = callbackFun;
		}

		virtual ~EventCallBack() override = default;
			
};

typedef std::list<std::unique_ptr<IEventCallBack>> HandlerList;

class EventBus
{
	private:
		std::map<std::type_index, std::unique_ptr<HandlerList>> subscribers;

	public:
		EventBus()
		{
			Logger::Log("EventBus Constructor");
		}

		~EventBus()
		{
			Logger::Log("EventBus Destructor");
		}

		//Clears subscriber list
		void Reset()
		{
			subscribers.clear();
		}


		//Subscribe to event of type T
		//listers subscribe to events by providing a callback function that will be executed when the event is emitted
		template <typename TEvent, typename TOwner>
		void SubscribeToEvent(TOwner* owner, void (TOwner::* callbackFun)(TEvent&))
		{
			if (!subscribers[typeid(TEvent)].get())
			{
				subscribers[typeid(TEvent)] = std::make_unique<HandlerList>();
			}
			auto subscriber = std::make_unique<EventCallBack<TOwner, TEvent>>(owner,callbackFun);
			subscribers[typeid(TEvent)]->push_back(std::move(subscriber));
		}

		//Emit event of type T
		// as soon as something emits an event we go ahead and execute all the listners callbackfun
		template<typename TEvent, typename ...TArgs>
		void EmitEvent(TArgs&& ...args)
		{
			auto handlers = subscribers[typeid(TEvent)].get();
			if(handlers)
				{
					for(auto it = handlers->begin(); it != handlers->end(); ++it)
					{
						auto handler = it->get();
						TEvent ev(std::forward<TArgs>(args)...);
						handler->Exectue(ev);
					}
				}
		}
};

#endif
