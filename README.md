# AMQP-CPP-xcode-example
A RabbitMQ message broker C11/C++ client Mac Xcode example project built on Copernica's AMQP-CPP library.

Sample work created to show my command of the C11/C++ language, demonstrate some good design practises and real-time development patterns/techniques.

This repo shows how to use Copernica’s AMQP-CPP C++ library to create a RabbitMQ client. The Xcode project contains 2 sample command line apps for message sender and consumer clients.

Everything which begins with “My” is my code. Everything in the AMQPSTL directory is code from Copernica packaged into a dynamic library for ease of Xcode on OS X development.

Some techniques demonstrated are:

Threading/performance considerations. I can see from the debugger that the user message receive callback from the Copernica library is being invoked in the context of the network read thread. So to avoid message processing holding up network writing in the RabbbitMQ server due to TCP flow control, messages are queued in the context of the network read rather than processed. The MyTaskProcessor takes a modern approach to queueing and processing in its own thread. See below for details on how I did this using std::packaged_task and std::async.

Similarly the debugger tells me that the Copernica API to send the message Ack involves a network write in the same thread context as the API call. So instead of potentially holding up the message processing by blocking on a network write, the sending of acks are also handled by another thread (via the MyAckProcessor class). See below for details on how std::future is waited on for sending the acks.

The Consumer app has command line options to either perform the above threading or process all messages and acks in the context of the user handler for benchmark comparison reasons. Benchmarking on the loopback does not show a performance improvement with the threaded option, however the loopback does not behave the same as the LAN/WAN. In the situation where a bursts of messages are arriving and being allowed to pile up on the task  and ack queues and message processing is blocking e.g. remote database operations, I would expect this state of affairs to outperform holding up network operations from the server and holding up processing on network writes.

If however I simulate a delay in sending acks with a small sleep then the threaded model processes the messages considerably faster due to the concurrent processing of messages and ack sending.

The impl or hidden implementation - implementation include files are confined to the impl and hidden from the wrapper class by use of a forward declaration for the impl. This reduces application dependencies, speeds application compilation and it is easy to make the wrapper object movable. See MyAMQPClientImpl and note that all the non 'impl' classes and include files are hidden from the application in this impl.

Smart pointers - std::unique_ptr is the first choice for efficiency and clear ownership reasons. Design based on the alternative std::shared_ptr can sometimes become a tangled model where ownership/responsibility is not clear to the maintainer and destruction order can be difficult to determine. However std::shared_ptr does have its uses. Usage of shared_ptr and weak_ptr is demonstrated in the MyCallbacksNotifier class which uses weak_ptr to model the situation where the ownership of the callbacks is outside of the client and temporary ownership for notification is required by the client.

The thread safe queue. This is a classic real time design pattern and allows a module/thread to have sole control over certain resources thus avoiding sharing resources across threads and ending up with convoluted deadlock prone code.

The thread safe queue and std::package_task. When operations on resources need to be performed by multiple threads the approach of sharing those resources and protecting contention with a mutex can lead to convoluted deadlock prone design. The classic thread safe queue model avoids this by allowing a subsystem to have sole control of those resources and be the reader of a queue containing messages that trigger operations on those resources. See MyReceiveTaskQueue and its use in MyAckProcessor and MyTaskProcessor in the MyAMQP directory of this project.

In the situation of where the sender of the message needs to receive a reply notification that the operation has completed the classic approach would provide a queue in the other direction. However, this approach can also lead to convoluted design. With C11 we can simplify this design pattern by queueing packaged tasks to be invoked by the reader of the queue. Before the sender of the task enqueues the std::packaged_task it can take a std::future and wait on it. When the queue reader pops the task from its queue and executes it the sender waiting on the future will get notified. Unlike the classic approach with queues in both directions the packaged_task allows the processor of the messages to be free of any handles to the sender.

Move semantics. Prior to C11 avoiding copying of objects whose ownership is transferred via parameters or method returns would require the use of pointers. The thread safe queue in this example avoids the need to have a queue of pointers to objects by providing a generic template on movable objects. C11 features such as std::future, std::packaged_task, std::thread lambdas etc are movable. However in the case of lambdas it can be neater to take a copy rather than move them. They are quite lightweight.

Async threading and futures. std::thread relieves us of the need to provide an OS abstraction layer over pthreads or windows threads when writing cross platform code. However, I found that there is generally no need to program with this low level object and instead std::async relieves us of the complexities of joining and simplifies exception catching. See Open and Close methods of MyAMQPBufferedConnection,  MyAckProcessor and MyTaskProcessor classes in the MyAMQP directory of this project.

Condition variables. Allows consistent waiting/notifiying of the predicate because the conditional wait is capable of testing the predicate and unlocking the mutex around the predicate when it goes into the wait in one atomic operation. See MyReceiveTaskQueue for the wait on a task to arrive, MyAMQPClientImpl on orderly queue creation and the Close() method.

Test driven development. Apart from the obvious of providing better testing, this methodology encourages  modular component design. The test driven coder soon realises that unit testing is facilitated by providing classes that can be tested in isolation without having to drag in heaps of dependencies and #include files. The pure abstract class or interface is one such technique - see MyNetworkInterface in the MyAMQP directory of this project. Passing of callbacks as arguments is also another technique. Visual studio has test projects as does Xcode for Swift and Objective-C, but not C++, hence my use of assert with an && “comment” in the directories beginning with “Test”.

Notes:
Copernica’s  AMQP-CPP repository compiled without modification except for adding the pragmas to export the symbols in “classes.h” from a dylib.

By default these clients connect to a local RabbitMQ server and send and receive from each other. Various command line options have been implemented.


