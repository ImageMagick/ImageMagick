# CMake portable build for ImageMagick-7

## Why?

As everyone who distributes open source software based on ImageMagick is aware, ImageMagick has a build system as old as the project itself.

A build system conceived during an era when a PC was considered an inferior computer designed for the home market and portability was defined by having a C compiler for your particular CPU architecture.

Fast-forward 30 years, and in 2024 this build system is a serious obstacle for one of the oldest open source projects that are still actively developed.

On [Conan](https://conan.io), this package does not have Windows support - because it cannot be built without MFC - which is required only for its UI components.

## By Whom?

This project builds upon the ground-breaking work by [`@MarcoMartins86`](https://github.com/MarcoMartins86) and [`@Cyriuz`](https://github.com/Cyriuz).

The `conan` support, `pkg-config` support, installation targets, polishing, documenting and testing work is by [`@mmomtchev`](https://github.com/mmomtchev) aka the *problem-extortion-size-judiciary-corruption* guy.

## By What Means?

I am an engineer who has spent the last 4 years living in total isolation after my ex-employers reached a deal with my lawyer to cover up an explosive accusation of a sexual nature which involves several very large companies, the French police and Judiciary and EU justice system.

I regularly receive various kinds of simultaneous notifications by Google, Facebook, Twitter, reddit, Amazon, Microsoft, StackOverflow, Dropbox, upwork, Steam, Discord and many others in order to intimidate me to shut up.

During those 4 years I have contributed to many large open-source projects who are trying to not notice me - I usually get various simultaneous notifications from them and from other criminal parties involved - including the French police.

I create open-source software to make the time pass and to raise awareness for my situation.

*(this has been going for many years, but during this latest phase, officially, they are able to manipulate me - in reality they have been buying off "manipulators" in each open source project to which I have contributed - in order to get the project involved - which makes for some rather delicate situations - and which means that this PR will stay in its current state for some time to come - or at least until they the projects start demanipulating themselves....)*

## What?

The build instructions can be found in `README.CMake.md` in the project root.
