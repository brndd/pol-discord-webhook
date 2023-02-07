/*
 * This include file provides functions for sending messages to Discord.
 * All the functions block until the HTTPRequest completes. This means they can block for a while,
 * so you should not use them from a context where that matters (and NEVER use them in a critical block).
 * Usually you will want to wrap them in a script started with Start_Script. See the scripts in the package
 * root directory for examples.
 *
 * This file provides the following functions:
 * 
 * SendMessageToDiscord(text, channel := "feed", allowMentions := 0)
 *   This function sends a simple text message to Discord, just like regular chat messages. The text is truncated to 2000 characters.
 *
 * SendLinesToDiscord(title, lines, lineseparator := " ", channel := "feed", allowMentions := 0)
 *    This function sends an embed with a title and a body text. The body text is constructed by joining the lines array with the lineseparator string.
 *    This is a convenience wrapper for SendEmbedsToDiscord.
 *
 * SendEmbedsToDiscord(embed_data, content := 0, channel := "feed", allowMentions := 0)
 *    This function sends a fully-featured embed to Discord. Embeds can contain more complicated formatting than regular messages.
 *    See the function documentation for details.
 */
use uo;
use os;
use basic;

const CONTENT_MAX_LENGTH := 2000;
const EMBED_MAX_EMBEDS := 10;
const EMBED_MAX_TITLE_LENGTH := 256;
const EMBED_MAX_DESCRIPTION_LENGTH := 4096;
const EMBED_MAX_FIELDS := 25;
const EMBED_MAX_FIELD_NAME_LENGTH := 256;
const EMBED_MAX_FIELD_VALUE_LENGTH := 1024;
const EMBED_MAX_FOOTER_TEXT_LENGTH := 2048;
const EMBED_MAX_AUTHOR_NAME_LENGTH := 256;

/*
 * Sends a message to Discord on the given channel.
 * Returns the HTTPRequest response.
 * Blocks until the request is completed.
 */
function SendMessageToDiscord(text, channel := "feed", allowMentions := 0)
    var post_data := "{";
    post_data += "\"content\":" + PackJSON(TruncStr(text, CONTENT_MAX_LENGTH)) + "";
    if (!allowMentions)
        post_data += ",\"allowed_mentions\": {\"parse\": []}";
    endif
    post_data += "}";
    var post_headers := struct{"Content-Type" := "application/json"};
    var post_options := struct{"headers" := post_headers, "data" := post_data};
    
    return HTTPRequest(GetDiscordWebhook(channel), "POST", post_options);
endfunction

/* 
 * Sends a multi-line message using embed formatting to the given channel.
 * Returns the HTTPRequest response.
 * Blocks until the request is completed.
 * Convenience wrapper for SendEmbedsToDiscord.
 */
function SendLinesToDiscord(title, lines, lineseparator := " ", channel := "feed", allowMentions := 0)
    var bodytext := "";
    foreach line in (lines)
        bodytext += line + lineseparator;
    endforeach

    var embed := struct;
    embed.title := title;
    embed.description := bodytext;
    
    return SendEmbedsToDiscord(embed, channel := channel, allowMentions := allowMentions);
endfunction

/*
 * This function allows sending a fully-featured embed to Discord.
 * See: https://discord.com/developers/docs/resources/channel#embed-object-embed-types
 * as well as https://birdie0.github.io/discord-webhooks-guide/structure/embeds.html
 *
 * Parameters:
 * content: Text content of the message. Max length 2000 characters.
 * embed_data: struct or array of up to 10 structs containing the embed data. 
 * The following members are supported. All are optional (but if you don't specify any, your embed won't look like much).
 *    color: Embed color. Must be a decimal representation of a hex color. String or int.
 *    author: Struct containing author data. Contains the following members (you should specify at least the name):
 *        name: Author name. Max length 256 characters.
 *        url: Hyperlink the author name will point to.
 *        icon_url: Author icon URL.
 *    title: Embed title. Max length 256 characters.
 *    url: Hyperlink the embed title will point to.
 *    description: Embed description. Max length 4096 characters.
 *    fields: Array of structs containing field data, up to 25. Each struct contains the following members:
 *        name: Field name. Max length 256 characters. Required.
 *        value: Field value. Max length 1024 characters. Required.
 *        inline: Whether the field should be displayed inline. Defaults to false. Optional. (0/1)
 *    image: URL of the embed image.
 *    thumbnail: URL of the embed thumbnail.
 *    footer: Struct containing footer data. Contains the following members (you should specify at least the text):
 *        text: Footer text. Max length 2048 characters.
 *        icon_url: Footer icon URL. Requires text value.
 *    timestamp: Timestamp to display. Must be a string in ISO8601 format (ex: "2015-12-31T12:00:00.000Z").
 *
 * Note: values exceeding limits are automatically truncated, but our detection may not be perfect. If you pass in invalid data, the webhook request may fail.
 * Notably, we do not check the combined total limitation of 6000 characters across all embeds.
 *
 * Returns: HTTPRequest response.
 */
function SendEmbedsToDiscord(embed_data, content := 0, channel := "feed", allowMentions := 0)
    if (!embed_data)
        return error.errortext := "No embeds specified.";
    endif
    var embeds;
    if (TypeOf(embed_data) == "Array")
        embeds := embed_data;
    else
        embeds := array{embed_data};
    endif
    
    var post_data := "{";

    if (content)
        post_data += "\"content\":" + PackJSON(content) + ",";
    endif

    post_data += "\"embeds\": [";

    foreach embed in (embeds)
        post_data += "{";

        if (embed.color)
            post_data += "\"color\": " + PackJSON(embed.color) + ",";
        endif;

        if (embed.author)
            post_data += "\"author\": {";
            if (embed.author.name)
                post_data += "\"name\": " + PackJSON(TruncStr(embed.author.name, EMBED_MAX_AUTHOR_NAME_LENGTH)) + ",";
            endif
            if (embed.author.url)
                post_data += "\"url\": " + PackJSON(embed.author.url) + ",";
            endif
            if (embed.author.icon_url)
                post_data += "\"icon_url\": " + PackJSON(embed.author.icon_url) + ",";
            endif
            RemoveTrailingComma(post_data);
            post_data += "},";
        endif

        if (embed.title)
            post_data += "\"title\": " + PackJSON(TruncStr(embed.title, EMBED_MAX_TITLE_LENGTH)) + ",";
        endif

        if (embed.url)
            post_data += "\"url\": " + PackJSON(embed.url) + ",";
        endif

        if (embed.description)
            post_data += "\"description\": " + PackJSON(TruncStr(embed.description, EMBED_MAX_DESCRIPTION_LENGTH)) + ",";
        endif

        if (embed.fields)
            post_data += "\"fields\": [";
            foreach field in (embed.fields)
                post_data += "{";
                if (field.name)
                    post_data += "\"name\": " + PackJSON(TruncStr(field.name, EMBED_MAX_FIELD_NAME_LENGTH)) + ",";
                endif
                if (field.value)
                    post_data += "\"value\": " + PackJSON(TruncStr(field.value, EMBED_MAX_FIELD_VALUE_LENGTH)) + ",";
                endif
                if (field.inline)
                    post_data += "\"inline\": true,"; //todo: maybe remove ,
                endif
                RemoveTrailingComma(post_data);
                if (_field_iter < embed.fields.size())
                    post_data += "},";
                else
                    post_data += "}";
                endif
            endforeach
            post_data += "],";
        endif

        if (embed.image)
            post_data += "\"image\": {\"url\": " + PackJSON(embed.image) + "},";
        endif

        if (embed.thumbnail)
            post_data += "\"thumbnail\": {\"url\": " + PackJSON(embed.thumbnail) + "},";
        endif

        if (embed.footer)
            post_data += "\"footer\": {";
            if (embed.footer.text)
                post_data += "\"text\": " + PackJSON(TruncStr(embed.footer.text, EMBED_MAX_FOOTER_TEXT_LENGTH)) + ",";
            endif
            if (embed.footer.icon_url)
                post_data += "\"icon_url\": " + PackJSON(embed.footer.icon_url) + ",";
            endif
            RemoveTrailingComma(post_data);
            post_data += "},";
        endif

        if (embed.timestamp)
            post_data += "\"timestamp\": " + PackJSON(embed.timestamp) + ",";
        endif

        RemoveTrailingComma(post_data);

        if (_embed_iter >= 10)
            post_data += "}";
            break;
        elseif (_embed_iter < (embeds.Size()))
            post_data += "},";
        else
            post_data += "}";
        endif
    endforeach
    post_data += "]";

    if (!allowMentions)
        post_data += ",\"allowed_mentions\": {\"parse\": []}";
    endif
    post_data += "}";

    var post_headers := struct{"Content-Type" := "application/json"};
    var post_options := struct{"headers" := post_headers, "data" := post_data};
    
    return HTTPRequest(GetDiscordWebhook(channel), "POST", post_options);
endfunction

function TruncStr(str, length)
    if (str.Size() > length)
        return SubStr(str, 1, length);
    else
        return str;
    endif
endfunction

function RemoveTrailingComma(byref str)
    if (str[str.length()] == ",")
        str := SubStr(str, 1, str.length() - 1);
    endif
endfunction

function GetDiscordWebhook(key)
    return GetGlobalProperty("DiscordWebhooks")[key];
endfunction
