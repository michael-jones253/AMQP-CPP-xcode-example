# AMQP-CPP-xcode-example
A RabbitMQ message broker C11/C++ client Mac Xcode example project built on Copernica's AMQP-CPP library.

Sample work created to show my command of the C11/C++ language, demonstrate some good design practises and real-time development patterns/techniques.

This repo shows how to use Copernica’s AMQP-CPP C++ library to create a RabbitMQ client. The Xcode project contains 2 sample command line apps for message "Sender" and "Consumer" clients.

Everything which begins with “My” is my code. Most of my interesting stuff described below is in the MyAMQP directory. Everything in the AMQPSTL directory is code from Copernica packaged into a dynamic library for ease of Xcode on OS X development.

Some techniques demonstrated are:

## Threading/performance considerations.
I can see from the debugger that the user message receive callback from the Copernica library is being invoked in the context of the network read thread. So to avoid message processing in the client holding up network writing in the RabbbitMQ server due to TCP flow control, messages are queued in the context of the network read rather than processed. The MyTaskProcessor class takes a modern approach to queueing and processing in its own thread. See below for details on how I did this using std::packaged_task and std::async.

Similarly the debugger tells me that the Copernica API to send the message Ack involves a network write in the same thread context as the API call. So instead of potentially holding up the message processing by blocking on a network write, the sending of acks are also handled by another thread (via the MyAckProcessor class). See below for details on how std::future is waited on for sending the acks.

The Consumer app has command line options to either perform the above threading or process all messages and acks in the context of the user handler for benchmark comparison reasons. Benchmarking on the loopback does not show a performance improvement with the threaded option, however the loopback does not behave the same as the LAN/WAN. In the situation where a bursts of messages are arriving and being allowed to pile up on the task  and ack queues and message processing is blocking e.g. remote database operations, I would expect this state of affairs to outperform the simple model which holds up network operations from the server and holds up message processing on the network writes.

If I simulate a network delay in sending acks with a small sleep on each ack write, then running the app in threaded mode processes the messages considerably faster. This is due to the concurrent processing of messages and ack sending. Using a benchmark of 1000 messages and a delay of 1 millisecond per ack the threaded model runs 10 times faster!

## The impl or hidden implementation.
Implementation include files are confined to the impl and hidden from the wrapper class by use of a forward declaration for the impl. This reduces application dependencies, speeds application compilation and it is easy to make the wrapper object movable. See MyAMQPClientImpl in the MyAMQP directory and note that all the non 'impl' classes and include files are hidden from the application in this impl. Also see MySignalHandler in the MyUtilities directory for how an impl can be used to make platform specific variants.

Another style of impl I have used is the POD (plain old data) which is just a hidden struct and all control flow logic is held in the owning class. The POD style impl is not suited to threaded situations where move operations are needed. See MyUnixNetworkConnection in the MyNetworkConnection directory for an example of this 

## Smart pointers.
std::unique_ptr is the first choice for efficiency and clear ownership reasons. Design based on the alternative std::shared_ptr can sometimes become a tangled model where ownership/responsibility is not clear to the maintainer and destruction order can be difficult to determine. However std::shared_ptr does have its uses. Usage of shared_ptr and weak_ptr is demonstrated in the MyCallbacksNotifier class which uses weak_ptr to model the situation where the ownership of the callbacks is outside of the client and temporary ownership for notification is required by the client.

## The thread safe queue.
This is a classic real time design pattern and allows a module/thread to have sole control over certain resources thus avoiding sharing resources across threads and ending up with convoluted deadlock prone code.

The thread safe queue and a modern approach with std::package_task. If you are not familiar with packaged tasks then see my explanation at the end of this readme.

When operations on resources need to be performed by multiple threads the approach of sharing those resources and protecting contention with a mutex can lead to convoluted deadlock prone design. The classic thread safe queue model avoids this by allowing a subsystem to have sole control of those resources and be the reader of a queue containing messages that trigger operations on those resources. See MyReceiveTaskQueue and its use in MyAckProcessor and MyTaskProcessor in the MyAMQP directory of this project.

In the situation of where the sender of the message needs to receive a reply notification that the operation has completed the classic approach would provide a queue in the other direction. However, this approach can also lead to convoluted design. With C11 we can simplify this design pattern by queueing packaged tasks to be invoked by the reader of the queue. Before the sender of the task enqueues the std::packaged_task it can take a std::future and wait on it. When the queue reader pops the task from its queue and executes it the sender waiting on the future will get notified. Unlike the classic approach with queues in both directions the packaged_task allows the processor of the messages to be free of any handles to the sender.

## Move semantics.
Prior to C11 avoiding copying of objects whose ownership is transferred via parameters or method returns would require the use of pointers. The thread safe queue in this example avoids the need to have a queue of pointers to objects by providing a generic template on movable objects. C11 features such as std::future, std::packaged_task, std::thread lambdas etc are movable. However in the case of lambdas it can be neater to take a copy rather than move them. They are quite lightweight. See MyAMQPClient in the MyAMQP client for how an impl makes writing move operations simple.

## Async threading and futures.
std::thread relieves us of the need to provide an OS abstraction layer over pthreads or windows threads when writing cross platform code. However, I found that there is generally no need to program with this low level object and instead std::async relieves us of the complexities of joining and simplifies exception catching. See Open and Close methods of MyAMQPBufferedConnection,  MyAckProcessor and MyTaskProcessor classes in the MyAMQP directory of this project.

## Condition variables.
Allows consistent waiting/notifiying of the predicate because the conditional wait is capable of testing the predicate and unlocking the mutex around the predicate when it goes into the wait in one atomic operation. See MyReceiveTaskQueue for the wait on a task to arrive, MyAMQPClientImpl on orderly queue creation and the Close() method.

## Test driven development.
Apart from the obvious of providing better testing, this methodology encourages  modular component design. The test driven coder soon realises that unit testing is facilitated by providing classes that can be tested in isolation without having to drag in heaps of dependencies and #include files. The pure abstract class or interface is one such technique - see MyNetworkInterface in the MyAMQP directory of this project. Passing of callbacks as arguments is also another technique.

Abstraction via an interface or generic callbacks allows the test program to provide dummy abstractions to the component being tested.

Another advantage of test driven development is that it can be quicker to get something working. Particularly with some client server situations setting up the conditions to test new functionality in situ can be a waste of time compared to setting it up via a unit test.

Visual studio has test projects as does Xcode for Swift and Objective-C, but not C++, hence my use of assert with an && “comment” in the directories underneath “Test”.

## Minimise use of singletons.
Generally these are BAD, because they are global data which make it very difficult for the coder to determine destruction order of such data - sometimes with unexpected program behaviour on exit. They can also lead to a convoluted object ownership model - similar problem to a design based on shared_ptr (see above).

However, in the MyUtilities directory I have used the singleton pattern for a signal handler which by its nature is a singleton operation. To avoid the unclear destruction order problem that singletons suffer from I have used the RAII design pattern. The destruction of the signal callbacks is on the stack and deterministic and their destructor releases the installed signal handlers and restores system default handling. Thus ensuring that when our handlers go out of scope, system defaults take over. See MySignalCallbacks in the MyUtilities directory.

## Notes:
Copernica’s  AMQP-CPP repository compiled without modification except for adding the pragmas to export the symbols in “classes.h” from a dylib.

By default these clients connect to a local RabbitMQ server and send and receive from each other. Various command line options have been implemented. The consumer client can be paused with Ctrl-C and resumed without losing any messages. The consumer can also be disconnected and reconnected to the RabbitMQ server by sending SIGHUP (signal hangup). This has allowed me to test the robustness of the open close pattern and the ability to stop and start modules by terminating and recreating the std::async threads associated with the module.

I have tested closing and opening under load by using the --count <message count> option with a large number on the client to send a flood of messages. These features added extra complexity to this project, but part of the motivation for this is to have some fun. In a commercial product, unecessary complexity should be avoided.

## An explanation of packaged tasks.

What is a packaged task? It is a bundle of work that can be ran asynchronously i.e the dispatcher can get that bundle of work run without blocking and wait for or get the result asynchronously (the future).

What is this bundle of work? It is a method/function that can be run - it could be a loop that runs for a long time or just a short execution. However this method needs to be associated with some data i.e it needs to be bound to some data or parameters. The binding of method to data can be done with std::bind or by using a lambda as the method.

Example usage might be to launch a number of tasks one after another without blocking on each launch, then wait for the results all in one go. Another pattern would be to have a different thread from the launcher wait for the result.

std::async class seems to do this? However the difference between std::async and std::packaged task is that the packaged task is just a bundle of work which does not include a thread for running it. Packaged tasks need to be handed to a thread to get the work run. Whereas std::async comes with a launch method that will run the bundle of work in a dedicated thread.

The programmer therefore needs to decide in what thread the packaged task will be run which makes it more flexible than std::async. e.g. a single thread can run many packaged tasks in a serialised manner whereas std::async is a associated with a thread 1:1 which will terminate once the future is delivered. If the design has a 1:1 association of work with thread then std::async will be the more convenient choice.

The biggest use that I have found for packaged tasks is queuing them to a single thread for serialised processing. This example project actually contains a queued packaged task processor which I haven't mentioned above. It is the class called MyRequestProcessor. I didn't mention it because it is not part of the threading/performance considerations, but rather a necessity or by-product of Copernica's library. Basically Copernica's library is not thread safe, so if I am making calls to the library from different thread contexts it needs to be locked. A mutex around calls would be ugly and deadlock prone. Instead I package each request to the library into a packaged task and send it to MyRequestProcessor for serialised processing, thus completely avoiding any risk of deadlocking. I have used this technique successfully on commercial code.


