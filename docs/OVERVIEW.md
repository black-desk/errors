# Overview

When you are writing C++ code in a project where exceptions are not allowed,
error reporting often becomes a problem.
In the traditional model, in such cases,
developers almost only have error codes as the error handling mechanism.
However, handling errors based on error codes is relatively primitive
and not very user-friendly,
as the reported errors lack specific contextual information.
Troubleshooting requires detailed log inspection, which is inconvenient.

The error handling mechanism based on returned errors
in Golang (https://pkg.go.dev/errors)
has become quite usable after some time of development.

So let's bring the mechanism to c++.
