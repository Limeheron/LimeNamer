import Foundation

final class RESX_READER: NSObject, XMLParserDelegate
{
    private(set) var entries: [(key: String, value: String)] = []
    private var key: String? = nil
    private var value: String? = nil

    static func read(_ url: URL) throws -> [(key: String, value: String)]
    {
        let reader = RESX_READER()
        guard let parser = XMLParser(contentsOf: url) else {
            throw CocoaError(.fileReadNoSuchFile)
        }
        parser.delegate = reader
        guard parser.parse() else {
            throw CocoaError(.fileReadCorruptFile)
        }
        return reader.entries
    }

    func parser(_ parser: XMLParser, didStartElement element: String, namespaceURI: String?,
        qualifiedName: String?, attributes: [String: String] = [:])
    {
        if element == "data" {
            key = attributes["name"]
        } else if element == "value" && key != nil {
            value = ""
        }
    }

    func parser(_ parser: XMLParser, foundCharacters string: String)
    {
        if value != nil {
            value! += string
        }
    }

    func parser(_ parser: XMLParser, didEndElement element: String, namespaceURI: String?,
        qualifiedName: String?)
    {
        if element == "value", let key, let value {
            entries.append((key, value))
            self.value = nil
        } else if element == "data" {
            key = nil
        }
    }
}
