using MQTTnet.Client.Extensions;
using System.Text.RegularExpressions;

namespace MQTTnet.Client.Extensions;

internal static class StringToDictionaryExtension
{
    internal static IDictionary<string, string> ToDictionary(this string valuePairString, char kvpDelimiter, char kvpSeparator)
    {
        if (string.IsNullOrWhiteSpace(valuePairString))
        {
            return new Dictionary<string, string>();
        }

        IEnumerable<string[]> parts = new Regex($"(?:^|{kvpDelimiter})([^{kvpDelimiter}{kvpSeparator}]*){kvpSeparator}")
            .Matches(valuePairString)
            .Cast<Match>()
            .Select(m => new string[] {
                m.Result("$1"),
                valuePairString[
                    (m.Index + m.Value.Length)..(m.NextMatch().Success ? m.NextMatch().Index : valuePairString.Length)]
            });

        if (!parts.Any() || parts.Any(p => p.Length != 2))
        {
            return new Dictionary<string, string>();
        }

        IDictionary<string, string> map = parts.ToDictionary(kvp => kvp[0], (kvp) => kvp[1], StringComparer.OrdinalIgnoreCase);

        return map;
    }
}
