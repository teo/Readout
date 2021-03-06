Readout is the executable reading out the data from readout cards on the FLPs.


# Architecture

Readout is a multi-threaded process implementing a data flow connecting
together threads of different types through FIFO buffers.
Each thread runs in an independent loop. A main thread takes care to distribute data between threads and to
synchronize them on process startup and shutdown.
The actual runtime components are instanciated on startup from a configuration file.

The basic workflow consists of several steps:
1) data is produced by readout equipments
2) a data aggregator aggregates their output based on common data block IDs
3) result is distributed to consumers. Data is shared, each consumer may use (read-only)
the incoming data before releasing it. Pushing data to consumers is by default a blocking operation
(if FIFO full, readout waits until being able to push).

Backpressure is applied upstream between the readout threads
(output FIFO of step N-1 is not emptied any more when input FIFO of step N full).


## Readout equipments

Keeping the same vocabulary as in ALICE DAQ DATE software,
a readout equipment is a data source (typically, from a hardware device).
It is a thread running in a loop, populating an output FIFO with new data.

The base class ReadoutEquipment is derived in different types:
- ReadoutEquipmentDummy : a dummy software generator to push data to
memory without hardware readout card.
- ReadoutEquipmentRORC : the readout class able to readout CRORC and CRU
devices, using the ReadoutCard library DmaChannelInterface for readout.
- ReadoutEquipmentCruEmulator : a class emulating CRU data, with realistic
LHC clock rates.


## Aggregator

This is a simple loop putting in the same vector data with matching ID.
It expected for each equipment to have a monotonic increase of ID (but not necesseraly
continuous numbering).


## Consumers

The consumers are threads making use of the data. The following have been implemented:
- ConsumerStats : keeps count of number and size of blocks produced
by readout. Counters can be published to O2 Monitoring system.
- ConsumerFileRecorder : writes the readout data to a file
- ConsumerDataChecker : checks data content (header, payload). Implemented for
CRU internal data generator.
- ConsumerDataSampling : pushes data through the DataSampling interface
- ConsumerFMQ : pushes data outside readout process as a FairMQ device.
- ConsumerFairMQChannel : pushes data outside readout process as a FairMQ channel - with the WP5 format.
  This consumer may also create shared memory banks (see Memory management) to be used by equipments.

They all follow the interface defined in the base Consumer Class.


# Memory management

Readout creates on startup some banks (big blocks of memory),
which are then used to create pools of data pages which are filled at runtime
by the readout equipments (and put back in the same pool after use).

The memory layout is explicitely defined in the configuration file.

In practice, you will define one or more memory blocks to be used
by the equipments. Each block is configured in a section named [bank-...]
(e.g. [bank-a1]), specifying its type (e.g. type=malloc or type=MemoryMappedFile),
size (e.g. size=256M or size=4G) and optionally NUMA node to be used
(e.g. numaNode=1).

The special consumer 'FairMQChannel' may also create a memory bank,
allocated from the FMQ "unmanaged shared memory" feature, before the other banks
are created (and hence, being the first one, being used by default by equipments).

Each equipment will then create its private data pages pool from
a given bank. This is done in the corresponding equipment configuration section
with number (memoryPoolNumberOfPages=1000) and size of each page (memoryPoolPageSize=512k),
and which bank to use (memoryBankName=bank-a1). By default of a bank name,
readout will try to create the pool from the first bank available.
Several memory pools can be created from the same memory bank, if space allows.


# Data format
Readout uses the data format defined in FlpPrototype for internal indexing of the pages.
The content (payload) of the pages is not affected, and follows the RDH specification
if using a CRU equipment.

The (internal) base data type is a vector of header+payload pairs.
In practice, it deals with different object types:
- DataBlock : header+payload pair
- DataBlockContainer : object storing a DataBlock, specialized depending on
underlying MemPool, with ad-hoc release callback.
- DataSet : a vector of DataBlockContainer
- DataSetReference : a shared pointer to a DataSet object


# Configuration

Readout is configured with a ".ini"-formatted file. A documented example file is provided with the source code
and distribution. Each readout component is configured in a different file section.
The section name is used to get the type of the component to be instanciated.
Equipments should be prefixed as [equipment-...].
Consumers should be prefixed as [consumer-...].
General settings are defined in section [readout].

A reference of all configuration parameters is defined in the configurationParameters.md file.

To setup a new readout configuration starting from the provided file, the basic steps are:
- define a memory layout suitable for your readout.
- define the equipments to be used, and their parameters (in particular, the memory bank they should use.
If not specified, readout will use the first one available).



# Usage


At the moment, readout is a command-line utility only. It starts taking data when
launched, and stops when process exits (interactive CTRL+C, signal, or timeout
if configured).
It will be integrated with the O2 control system when available.

It takes as command-line argument the name of the configuration file to be used:

readout.exe file://path/to/my/readout.cfg

It may also be started by providing a URI for the O2 Configuration backend
(with as optional extra parameter the entry point in the configuration tree, by default empty, i.e. top of the tree)

readout.exe ini://path/to/my/readout.ini
readout.exe consul://localhost:8500 /readout



# Contact
sylvain.chapeland@cern.ch
