# POL Discord webhook package

This is a script package for [Penultima Online](https://polserver.com/) to facilitate sending
data to Discord webhooks. It provides a set of functions that make it easy to send messages
and complicated embed-formatted data.

This package was originally developed for the [Northern Winds](https://northern-winds.fi/) server.

# Usage

Install the package on your server. Edit [fileaccess.cfg](https://docs.polserver.com/pol100/configfiles.php#fileaccess.cfg)
to allow the package to access .json files in its own directory.

Modify [webhook-config.json](pkg/discord/webhook-config.json) to contain your webhook URLs.
You may also add extra name:url pairs here. They will be loaded into a global variable on the server
and can be accessed by their configuration name when calling the functions.

Use `include ":discord:sendMessage";` in your scripts to include the functions. See the comments in [sendMessage.inc](pkg/discord/include/sendMessage.inc)
for documentation of the functions.

**Important:** the functions are blocking; they won't return until the HTTP request finishes, which can take a long time,
and even in good conditions will probably take some milliseconds. Do not call them directly from scripts where speed is important.
*Especially* do not call them from critical sections or packet hooks.

If you do not want to wait for the functions to return, wrap them in a script that you start using [Start_Script](https://docs.polserver.com/pol100/fullfunc.php?xmlfile=osem#Start_Script).
This way the execution of your main script won't block. See [exampleMessage.src](pkg/discord/exampleMessage.src) and [exampleEmbed.src](pkg/discord/exampleEmbed.src) for examples.

Here's an example of what's possible with `SendEmbedsToDiscord`:

![](https://i.imgur.com/gxbOy9u.png)

# TODO

- Expand mentions configuration to allow more granularity than just allow/disallow.
- Use https://github.com/polserver/polserver/pull/504 to handle status codes once it lands in trunk.
- Allow specifying avatar and name on a per-message basis.
