# EMPR2

***University of York, Computer Science BEng/MEng Stage 2 EMPR (Embedded Systems Project) Part 2***

***Team Members**: Jacob Allen, Kia Alderson, Maxime Franchot, and Calum Ferguson*

## Contents

- [1. Project Overview](#1-project-overview)
- [2. Project Hardware](#2-project-hardware)
- [3. Group Components](#3-group-components)
  - [3.1. Monitor](#31-monitor)
  - [3.2. Generator](#32-generator)
  - [3.3. Display](#33-display)
- [4. Individual Components](#4-individual-components)

## 1. Project Overview

The goal of the project was to design and create a system for monitoring and generating DMX (Digital Multiplex) data on a DMX network, which can be used to control DMX compatible lighting.

## 2. Project Hardware

The system uses an interface board, consisting of a SN75176A Differential Bus Transceiver and supporting electronics, to convert to and from DMX data. The interface board is connected to the system I/O Board, which is in turn connected to a LPC1768 micro-controller on an ARM Mbed board. Two functionally identical hardware interface boards were also constructed for the monitor and the generator.  

## 3. Group Components

The goal of the group project was to construct the 3 main components of the system, the monitor, generator, and display.

### 3.1. Monitor

The monitor receives and decodes DMX data and provides an interface to view the data.

The source for this can be found in the `Monitor` directory.

This component was mainly worked on by Kia and Jacob.

### 3.2. Generator

The generator encodes and sends DMX data and provides an interface to program the data to be sent.

The source for this can be found in the `Generator` directory.

This component was mainly worked on by Max and Calum.

### 3.3. Display

The display runs on a computer connected to the monitor and provides a more user-friendly way to view the received DMX data. The source for this can be found in the `Display` directory.

This component was mainly worked on by Kia and Jacob.

## 4. Individual Components

Each member of the group also had an individual project.

The source for these can be found in the `IndividualProjects` directory for Kia, Max, and Calum. The source for Jacob's individual project can be found in the [JMALego/EMPR-Individual-Project](https://github.com/JMAlego/EMPR-Individual-Project) repository.
