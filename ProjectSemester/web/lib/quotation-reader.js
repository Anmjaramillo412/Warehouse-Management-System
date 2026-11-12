/* ============================================================
   QUOTATION READER
   Turns the text of a supplier quotation (PDF text layer, German or
   English) into a draft of the quotation form. Pure functions - no
   DOM, no network - so it can be tested outside the browser.

   Usage (browser):
     const lines = QuotationReader.buildLines(pdfTextItems);
     const draft = QuotationReader.parseQuotation(lines, context);

   context = {
     fileName: "Angebot 2026-30164.pdf",
     suppliers: [{ name: "Clear & Clean GmbH" }, ...],
     materials: [{ id: "008-100001", name: "...", partNumbers: ["CC445"] }]
   }

   Everything it is not sure about is listed in draft.warnings (global)
   or line.warnings, so the form can mark it for review.
   ============================================================ */

(function (root) {

    "use strict";

    const UNIT = "(?:St\\u00fcck|Stk\\.?|St\\.?|pcs\\.?|PCS|pc|db|VE|ME|Pkg\\.?|Rolle|Rollen|Satz|Set|Pack|m|M|kg|KG|l|L)";

    const NUM = "\\d[\\d.,]*";

    // ------------------------------------------------------------
    // Text items (pdf.js) -> text lines
    // ------------------------------------------------------------

    // items: [{ s, x, y, w }] of ONE page, any order
    function buildLines(items) {

        const sorted = items
            .filter(i => i.s && i.s.trim() !== "")
            .slice()
            .sort((a, b) => (b.y - a.y) || (a.x - b.x));

        const rows = [];

        for (const item of sorted) {

            const last = rows[rows.length - 1];

            if (last && Math.abs(last.y - item.y) <= 2.5) {
                last.items.push(item);
            }
            else {
                rows.push({ y: item.y, items: [item] });
            }
        }

        return rows.map(row => {

            row.items.sort((a, b) => a.x - b.x);

            let text = "";

            let end = null;

            for (const item of row.items) {

                if (end !== null) {

                    const gap = item.x - end;

                    text += gap > 7 ? "   " : gap > 1 ? " " : "";
                }

                text += item.s;

                end = item.x + (item.w || 0);
            }

            return text.trim();
        });
    }

    // ------------------------------------------------------------
    // Numbers and dates
    // ------------------------------------------------------------

    function parseNumber(text) {

        if (text === undefined || text === null) {
            return NaN;
        }

        let s = String(text).replace(/\s/g, "");

        const dot = s.lastIndexOf(".");

        const comma = s.lastIndexOf(",");

        if (dot >= 0 && comma >= 0) {

            if (comma > dot) {
                s = s.replace(/\./g, "").replace(",", ".");
            }
            else {
                s = s.replace(/,/g, "");
            }
        }
        else if (comma >= 0) {

            s = s.replace(/,/g, ".");
        }
        else if (dot >= 0 && /^[1-9]\d{0,2}(\.\d{3})+$/.test(s)) {

            s = s.replace(/\./g, "");
        }

        return parseFloat(s);
    }

    function pad(n) {
        return String(n).padStart(2, "0");
    }

    function validISO(y, m, d) {

        return y >= 2000 && y <= 2100 && m >= 1 && m <= 12 &&
            d >= 1 && d <= 31 ? `${y}-${pad(m)}-${pad(d)}` : "";
    }

    // All dates in a text: [{ iso, index }]
    function findDates(text) {

        const found = [];

        const patterns = [
            { re: /(\d{4})[.\-\/](\d{1,2})[.\-\/](\d{1,2})/g, order: "ymd" },
            { re: /(\d{1,2})[.\/](\d{1,2})[.\/](\d{4})/g, order: "dmy" },
            { re: /(\d{1,2})-(\d{1,2})-(\d{4})/g, order: "dmy" }
        ];

        for (const { re, order } of patterns) {

            let m;

            while ((m = re.exec(text)) !== null) {

                const a = parseInt(m[1], 10);
                const b = parseInt(m[2], 10);
                const c = parseInt(m[3], 10);

                const iso = order === "ymd" ?
                    validISO(a, b, c) : validISO(c, b, a);

                if (iso && !found.some(f => f.index === m.index)) {
                    found.push({ iso: iso, index: m.index, length: m[0].length });
                }
            }
        }

        return found.sort((x, y) => x.index - y.index);
    }

    // ------------------------------------------------------------
    // Helpers
    // ------------------------------------------------------------

    function normalizeName(name) {

        return String(name || "")
            .toLowerCase()
            .replace(/&/g, " ")
            .replace(/\b(gmbh|mbh|kft|ltd|co|ag|kg|sro|s\.r\.o|inc|llc|handelsges|handelsgesellschaft|und|and)\b/g, " ")
            .replace(/[^a-z0-9äöüß ]/g, " ")
            .replace(/\s+/g, " ")
            .trim();
    }

    function normalizePN(pn) {
        return String(pn || "").toUpperCase().replace(/[\s\-._\/]/g, "");
    }

    function isFreightText(text) {
        return /fracht|versand|porto|verpackung|transport|freight|shipping|packaging|handling|\bFR\b/i.test(text);
    }

    // ------------------------------------------------------------
    // Header fields
    // ------------------------------------------------------------

    function findSupplier(lines, context, result) {

        const suppliers = context.suppliers || [];

        const haystack = normalizeName(
            lines.slice(0, 60).join(" ") + " " +
            lines.slice(-25).join(" ") + " " + (context.fileName || ""));

        let best = null;

        for (const supplier of suppliers) {

            const key = normalizeName(supplier.name);

            if (key.length < 4) {
                continue;
            }

            if (haystack.includes(key) &&
                (!best || key.length > best.key.length)) {

                best = { name: supplier.name, key: key };
            }
        }

        if (!best) {

            // first word of the name (e.g. "Fastlink")
            for (const supplier of suppliers) {

                const word = normalizeName(supplier.name).split(" ")[0];

                if (word.length >= 5 && haystack.includes(word) &&
                    (!best || word.length > best.key.length)) {

                    best = { name: supplier.name, key: word };
                }
            }
        }

        if (best) {
            result.supplier = best.name;
        }
        else {

            const guess = lines.slice(0, 40).find(line =>
                /\b(GmbH|Kft\.?|Ltd\.?|AG|s\.r\.o\.?|Inc\.?|S\.A\.|B\.V\.)/i.test(line) &&
                !/4Tex/i.test(line));

            result.supplierGuess = guess || "";

            result.warnings.push(
                "Supplier not recognized" +
                (guess ? " (the document says: " + guess.slice(0, 60) + ")" : "") +
                " - choose it.");
        }
    }

    function findNumber(lines, context, result) {

        const text = lines.join("\n");

        const labeled = [
            /Belegnummer\s*[:#]?\s*([A-Z0-9][\w\-\/\.]*)/i,
            /Kennzahl\s*[:#]?\s*([A-Z0-9][\w\-\/\.]*)/i,
            /A\s?N\s?G\s?E\s?B\s?O\s?T\s*(?:Nr\.?|Nummer|No\.?)\s*[:.]?\s*([A-Z0-9][\w\-\/\.]*)/i,
            /(?:Quotation|Quote|Offer)\s*(?:No\.?|Number|Nr\.?|#)\s*[:.]?\s*([A-Z0-9][\w\-\/\.]*)/i,
            /Angebots?-?\s*(?:Nr\.?|Nummer)\s*[:.]?\s*([A-Z0-9][\w\-\/\.]*)/i
        ];

        for (const re of labeled) {

            const m = text.match(re);

            if (m && /\d/.test(m[1])) {

                result.supplierQuoteNumber = m[1].replace(/[.,]$/, "");

                return;
            }
        }

        // The file name usually carries the number too
        const tokens = (context.fileName || "")
            .replace(/\.pdf$/i, "")
            .match(/[A-Z0-9][A-Z0-9\-\/]{4,}/gi) || [];

        for (const token of tokens) {

            if (/\d/.test(token) && !findDates(token).length &&
                text.includes(token)) {

                result.supplierQuoteNumber = token;

                return;
            }
        }

        for (const token of tokens) {

            if (/\d{4,}/.test(token) && !findDates(token).length) {

                result.supplierQuoteNumber = token;

                result.warnings.push(
                    "Quotation number taken from the file name - check it.");

                return;
            }
        }

        result.warnings.push("Quotation number not found.");
    }

    function findDatesFields(lines, result) {

        const enquiry = /anfrage|enquiry|inquiry|request/i;

        const validRe = /gültig|valid(?:ity)?\s*(?:until|to|through)?|gueltig|angebotsg/i;

        const issueLabel = /(angebotsdatum|belegdatum|datum|date)\b/i;

        let issue = "";

        let validUntil = "";

        for (let i = 0; i < lines.length; i++) {

            const line = lines[i];

            const dates = findDates(line);

            // Label line with several columns (Datum | Gültigkeitsfrist),
            // dates on the next line in the same order
            if (!dates.length && issueLabel.test(line) && validRe.test(line) &&
                lines[i + 1]) {

                const next = findDates(lines[i + 1]);

                const issuePos = line.search(issueLabel);

                const validPos = line.search(validRe);

                if (next.length >= 2) {

                    if (issuePos < validPos) {
                        issue = issue || next[0].iso;
                        validUntil = validUntil || next[1].iso;
                    }
                    else {
                        validUntil = validUntil || next[0].iso;
                        issue = issue || next[1].iso;
                    }
                }

                continue;
            }

            if (!dates.length) {
                continue;
            }

            if (validRe.test(line) && !validUntil) {

                // the date AFTER the validity words
                const pos = line.search(validRe);

                const after = dates.find(d => d.index > pos) || dates[0];

                validUntil = after.iso;

                continue;
            }

            if (!issue && issueLabel.test(line) && !enquiry.test(line)) {

                const pos = line.search(issueLabel);

                const after = dates.find(d => d.index > pos) || dates[0];

                issue = after.iso;
            }
        }

        if (!issue) {

            // first date that is not an enquiry / validity date
            for (const line of lines) {

                if (enquiry.test(line) || validRe.test(line)) {
                    continue;
                }

                const dates = findDates(line);

                if (dates.length) {

                    issue = dates[0].iso;

                    result.warnings.push(
                        "Issue date guessed (first date in the document) - check it.");

                    break;
                }
            }
        }

        result.issueDate = issue;

        // A quotation may have no validity date: no warning for it
        result.validUntil = validUntil;
    }

    function findCurrency(lines, result) {

        const text = lines.join(" ");

        const eur = (text.match(/EUR|€/g) || []).length;

        const usd = (text.match(/USD|US\$|\$/g) || []).length;

        result.currency = usd > eur ? "USD" : "EUR";

        if (eur === 0 && usd === 0) {
            result.warnings.push("Currency not found - EUR assumed.");
        }
    }

    function findLeadTime(lines, result) {

        const text = lines.join("\n");

        let weeks = 0;

        const patterns = [
            new RegExp("Lieferzeit(?:en)?\\s*[:]?\\s*(?:ca\\.?|circa|approx\\.?)?\\s*(\\d+)(?:\\s*[-–]\\s*(\\d+))?\\s*(Wochen?|Woche\\(n\\)|Tage?n?|weeks?|days?)", "gi"),
            /(\d+)(?:\s*[-–]\s*(\d+))?\s*(Woche\(n\)|Wochen|Woche)/gi,
            /lead\s*time\s*[:]?\s*(?:approx\.?|ca\.?)?\s*(\d+)(?:\s*[-–]\s*(\d+))?\s*(weeks?|days?)/gi
        ];

        for (const re of patterns) {

            let m;

            while ((m = re.exec(text)) !== null) {

                let value = parseInt(m[2] || m[1], 10);

                const unit = m[3].toLowerCase();

                if (/^t|^d/.test(unit)) {
                    value = Math.ceil(value / 7);
                }

                weeks = Math.max(weeks, value);
            }
        }

        result.leadTimeWeeks = weeks;

        if (weeks === 0) {
            result.warnings.push("No lead time found in the document.");
        }
    }

    function findIncoterm(lines, result) {

        const text = lines.join(" ");

        const m = text.match(/\b(EXW|FCA|FAS|FOB|CFR|CIF|CPT|CIP|DAP|DPU|DDP)\b/);

        if (m) {

            result.incoterm = m[1];
        }
        else if (/ab\s+werk|ex\s+works/i.test(text)) {

            result.incoterm = "EXW";
        }
        else {

            result.incoterm = "";
        }
    }

    // ------------------------------------------------------------
    // Item lines
    // ------------------------------------------------------------

    const POSITION = /^\s*(\d{1,3})\s*(?:\.|\/\d+)?\s+/;

    function articleTokens(text) {

        // tokens that look like an article number: contain a digit,
        // 3+ characters, not a unit/number-with-comma/date/KW
        return (text.match(/[A-Za-z0-9][A-Za-z0-9.\-\/]{2,}/g) || [])
            .filter(t =>
                /\d/.test(t) &&
                !/^\d{4}\/\d{2}$/.test(t) &&
                !/^\d+[.,]\d+$/.test(t) &&
                !/^\d{1,3}$/.test(t) &&
                !findDates(t).length);
    }

    function parseQuantityLine(text) {

        // qty unit unitprice [/ per unit] [total]
        const re = new RegExp("(" + NUM + ")\\s*(" + UNIT + ")(?![A-Za-z\\u00e4\\u00f6\\u00fc])", "g");

        let m;

        while ((m = re.exec(text)) !== null) {

            const qty = parseNumber(m[1]);

            const rest = text.slice(m.index + m[0].length);

            // price [/ N unit]  or  price PE
            const price = rest.match(new RegExp("^\\s*(?:EUR|\\u20ac|USD|\\$)?\\s*(" + NUM + ")\\s*(?:EUR|\\u20ac|USD|\\$)?(?:\\s*\\/\\s*(\\d+)\\s*" + UNIT + "?|\\s+(\\d{1,4})(?=\\s+" + NUM + "\\s*$))?"));

            if (qty > 0 && price) {

                const per = parseInt(price[2] || price[3] || "1", 10) || 1;

                const unitPrice = parseNumber(price[1]) / per;

                if (unitPrice > 0) {

                    return {
                        quantity: qty,
                        unit: m[2],
                        unitPrice: unitPrice,
                        index: m.index
                    };
                }
            }
        }

        return null;
    }

    function parseTierLines(lines, from, item) {

        let i = from;

        let inTable = false;

        for (; i < lines.length; i++) {

            const line = lines[i];

            if (/Rabatt|Preisnachlass|Bestellwert|Bestellsumme|discount|order value/i.test(line)) {
                continue;
            }

            // "250 oder weitere 0,1441 € / Stück"
            let m = line.match(new RegExp("(" + NUM + ")\\s*(?:" + UNIT + ")?\\s*(?:oder weitere|oder mehr|or more|und mehr|\\+)\\s*:?\\s*(?:EUR|\\u20ac|USD|\\$)?\\s*(" + NUM + ")"));

            if (m) {

                addTier(item, parseNumber(m[1]), parseNumber(m[2]));

                continue;
            }

            // "ab 100 Stück: 0,12 €"
            m = line.match(new RegExp("^\\s*(?:ab|from|\\u2265)\\s*(" + NUM + ")\\s*(?:" + UNIT + ")?\\s*[:=\\-]?\\s*(?:EUR|\\u20ac|USD|\\$)?\\s*(" + NUM + ")"));

            if (m && !/%/.test(line)) {

                addTier(item, parseNumber(m[1]), parseNumber(m[2]));

                continue;
            }

            // "Staffelpreise ...   10,00   125,34" and following pairs
            if (/Staffelpreis|price breaks|quantity discount|Mengenstaffel/i.test(line)) {

                inTable = true;

                const tail = line.match(new RegExp("(" + NUM + ")\\s+(" + NUM + ")\\s*$"));

                if (tail) {
                    addTier(item, parseNumber(tail[1]), parseNumber(tail[2]));
                }

                continue;
            }

            if (inTable) {

                const pair = line.match(new RegExp("^\\s*(" + NUM + ")\\s+(" + NUM + ")\\s*$"));

                if (pair) {

                    addTier(item, parseNumber(pair[1]), parseNumber(pair[2]));

                    continue;
                }

                if (line.trim() === "") {
                    continue;
                }

                inTable = false;
            }

            const moq = line.match(new RegExp("(?:Mindestbestellmenge|Mindestmenge|MOQ|Min\\.?\\s*order\\s*quantity)\\s*[:=]?\\s*(" + NUM + ")"));

            if (moq) {

                item.moq = parseNumber(moq[1]);

                continue;
            }

            // next position starts: stop
            if (POSITION.test(line) && /\d/.test(line) && parseQuantityLine(line)) {
                break;
            }

            if (/Zwischensumme|Netto|Summe|Endsumme|Total|Gesamt|Seite|Page|Übertrag/i.test(line) &&
                !item.stopSeen) {

                item.stopSeen = true;
            }
        }
    }

    function addTier(item, minQuantity, unitPrice) {

        if (!(minQuantity > 0) || !(unitPrice > 0)) {
            return;
        }

        if (!item.tiers.some(t => t.minQuantity === minQuantity)) {
            item.tiers.push({ minQuantity: minQuantity, unitPrice: unitPrice });
        }
    }

    function parseItems(lines, context, result) {

        const items = [];

        const byKey = {};

        const notes = [];

        let pending = null;

        for (let i = 0; i < lines.length; i++) {

            const line = lines[i];

            if (/Rabatt|Preisnachlass|Mindestbestellwert|Bestellwert|Bestellsumme|Zahlungsbedingung|Zahlung:|Vorkasse|payment terms/i.test(line) &&
                /\d/.test(line) && !/wie folgt|Seite|Page|AGB/i.test(line) &&
                notes.length < 8 && line.length < 140) {

                notes.push(line.replace(/\s{2,}/g, " "));
            }

            // Freight / packaging / shipping amounts go to the notes
            // (they are not materials)
            if (/Fracht|Porto|Versandkosten|Verpackung|Transportkosten|freight|shipping cost|packaging/i.test(line) &&
                !/Versandtermin|Versandart/i.test(line)) {

                const own = line.match(new RegExp("(" + NUM + ")\\s*(?:EUR|\u20ac|USD|\\$)?\\s*$"));

                const nextLine = lines[i + 1] || "";

                const nextMoney = !own && !/\d/.test(line.replace(POSITION, "").replace(/\b(FR|Fr)\b/, "")) ?
                    nextLine.match(new RegExp("(" + NUM + ")\\s*(?:EUR|\u20ac)")) : null;

                const label = (line.match(/Fracht|Porto|Versandkosten|Verpackung|Transportkosten|freight|shipping cost|packaging/i) || ["Freight"])[0];

                const amount = own ? own[1] : (nextMoney ? nextMoney[1] : "");

                if (amount && /\d,\d\d|\d\.\d\d/.test(amount)) {
                    notes.push(label + " " + amount + " EUR");
                }

                pending = null;

                continue;
            }

            // A tier line ("500 oder weitere 0,1145 EUR") is not a position
            const position = /oder weitere|or more|Staffelpreis/i.test(line) ?
                null : line.match(POSITION);

            let header = null;

            let quantityLine = null;

            if (position) {

                const q = parseQuantityLine(line);

                if (q) {

                    header = line;

                    quantityLine = q;
                }
                else if (articleTokens(line).length > 0 &&
                    !/Pos\.?\s|Position/.test(line)) {

                    // Position + article, quantity on a following line
                    pending = { text: line, index: i };

                    continue;
                }
            }
            else if (pending && i - pending.index <= 3) {

                const q = parseQuantityLine(line);

                if (q) {

                    header = pending.text;

                    quantityLine = q;

                    pending = null;
                }
            }

            if (!quantityLine) {
                continue;
            }

            const headerText = header;

            if (isFreightText(headerText) && !/Stk|Stück/.test(headerText)) {

                const money = line.match(new RegExp("(" + NUM + ")\\s*(?:EUR|\\u20ac)\\s*$"));

                if (money) {
                    notes.push((headerText.match(/Fracht\w*|Porto\w*|Versand\w*|Verpackung\w*/i) || ["Freight"])[0] +
                        " " + money[1] + " EUR");
                }

                continue;
            }

            // article tokens (before the quantity on the header line)
            const beforeQty = header === line ?
                line.slice(0, quantityLine.index) : header;

            const tokens = articleTokens(
                beforeQty.replace(POSITION, ""));

            if (tokens.length === 0) {
                continue;
            }

            // description: text after the article tokens, else next line
            let description = beforeQty
                .replace(POSITION, "")
                .replace(/\s+\d{4}\/\d{2}\s*$/, "")
                .trim();

            for (const t of tokens) {
                description = description.replace(t, " ");
            }

            description = description.replace(/\s{2,}/g, " ").trim();

            if (description.length < 3) {

                const next = lines[i + 1] || "";

                description = POSITION.test(next) ? "" : next.trim();
            }

            const key = normalizePN(tokens[0]);

            let item = byKey[key];

            if (!item) {

                item = {
                    articleNumbers: tokens,
                    description: description,
                    moq: 0,
                    usualQuantity: 0,
                    tiers: [],
                    quotedQuantities: [],
                    textTiers: false,
                    warnings: []
                };

                byKey[key] = item;

                items.push(item);
            }

            item.quotedQuantities.push(quantityLine.quantity);

            const before = item.tiers.length;

            parseTierLines(lines, i + 1, item);

            if (item.tiers.length > before) {
                item.textTiers = true;
            }

            if (!item.textTiers) {
                addTier(item, quantityLine.quantity, quantityLine.unitPrice);
            }
        }

        result.notes = notes;

        // finishing: sort tiers, usual quantity, material match
        for (const item of items) {

            item.tiers.sort((a, b) => a.minQuantity - b.minQuantity);

            // The quantity that was asked for is the usual one - only
            // when there is exactly one and it is a tier
            const asked = Array.from(new Set(item.quotedQuantities));

            if (asked.length === 1 &&
                item.tiers.some(t => t.minQuantity === asked[0])) {

                item.usualQuantity = asked[0];
            }

            delete item.quotedQuantities;

            delete item.textTiers;

            delete item.stopSeen;

            matchMaterial(item, context, result);
        }

        return items;
    }

    function matchMaterial(item, context, result) {

        const wanted = item.articleNumbers.map(normalizePN);

        for (const material of (context.materials || [])) {

            for (const pn of (material.partNumbers || [])) {

                if (pn && wanted.includes(normalizePN(pn))) {

                    item.materialID = material.id;

                    item.materialName = material.name || "";

                    return;
                }
            }
        }

        item.materialID = "";

        item.warnings.push(
            "No material has supplier article number " +
            item.articleNumbers.join(" / ") +
            " - choose the material" +
            (item.description ? " (" + item.description.slice(0, 50) + ")" : "") + ".");
    }

    // ------------------------------------------------------------
    // Price table pasted from a web shop (RS, Mouser, ...)
    // ------------------------------------------------------------

    // "10 - 240   € 0,086   € 0,86"  /  "10000 +  € 0,051"  /
    // "1  € 143,58  € 143,58"        /  "500  Quote" (skipped)
    function parseTierTable(text) {

        const tiers = [];

        for (const raw of String(text || "").split(/\r?\n/)) {

            const line = raw.replace(/ /g, " ").trim();

            if (!line || /quote|anfrage|angebot|request/i.test(line)) {
                continue;
            }

            const m = line.match(new RegExp("^(\\d{1,3}(?:,\\d{3})+|\\d[\\d.\\s]*)\\s*(?:[-\\u2013]\\s*\\d[\\d.\\s]*|\\+|bis|to)?\\s*(?:\\u20ac|EUR|\\$|USD)?\\s*(" + NUM + ")"));

            if (!m) {
                continue;
            }

            // "1,000" (English thousands) as well as "1.000" / "1 000"
            let quantityText = m[1].trim();

            if (/^\d{1,3}(,\d{3})+$/.test(quantityText)) {
                quantityText = quantityText.replace(/,/g, "");
            }

            const quantity = parseNumber(quantityText);

            const price = parseNumber(m[2]);

            if (quantity > 0 && price > 0 &&
                !tiers.some(t => t.minQuantity === quantity)) {

                tiers.push({ minQuantity: quantity, unitPrice: price });
            }
        }

        return tiers.sort((a, b) => a.minQuantity - b.minQuantity);
    }

    // Tiers plus plausibility checks, used for text read from a
    // PICTURE (OCR can misread a digit): prices must not rise with
    // the quantity, and when the table also has a per-pack column
    // ("Pro Packung") it must be the unit price x one constant pack
    // size on every row.
    function analyzeTierTable(text) {

        const tiers = parseTierTable(text);

        const warnings = [];

        if (tiers.length === 0) {
            return { tiers: tiers, warnings: warnings };
        }

        if (tiers.length === 1) {
            warnings.push("Only one price tier was read - check the picture.");
        }

        for (let i = 1; i < tiers.length; i++) {

            if (tiers[i].unitPrice > tiers[i - 1].unitPrice) {

                warnings.push("The price rises at quantity " +
                    tiers[i].minQuantity + " - one number may be misread.");
            }
        }

        // second price column
        const ratios = [];

        for (const raw of String(text || "").split(/\r?\n/)) {

            const line = raw.replace(/\u00a0/g, " ").trim();

            if (!line || /quote|anfrage|angebot|request/i.test(line)) {
                continue;
            }

            const m = line.match(new RegExp("^(\\d{1,3}(?:,\\d{3})+|\\d[\\d.\\s]*)\\s*(?:[-\\u2013]\\s*\\d[\\d.\\s]*|\\+|bis|to)?\\s*(?:\\u20ac|EUR|\\$|USD)?\\s*(" + NUM + ")\\s*(?:\\u20ac|EUR|\\$|USD)?\\s*(" + NUM + ")?\\s*$"));

            if (m && m[3]) {

                const unit = parseNumber(m[2]);

                // OCR often drops the comma of the small pack price
                // ("641" for "6,41")
                const pack = /^\d{3,}$/.test(m[3]) ?
                    parseInt(m[3], 10) / 100 : parseNumber(m[3]);

                if (unit > 0 && pack > 0) {
                    ratios.push({ quantity: parseNumber(m[1].trim()), ratio: pack / unit });
                }
            }
        }

        if (ratios.length >= 3) {

            const sorted = ratios.map(r => r.ratio).sort((a, b) => a - b);

            const median = sorted[Math.floor(sorted.length / 2)];

            for (const r of ratios) {

                if (Math.abs(r.ratio - median) / median > 0.02) {

                    warnings.push("Quantity " + r.quantity +
                        ": the unit price and the pack price do not agree - check this row.");
                }
            }
        }

        return { tiers: tiers, warnings: warnings };
    }

    // ------------------------------------------------------------
    // Whole quotation
    // ------------------------------------------------------------

    function parseQuotation(lines, context) {

        context = context || {};

        const result = {
            supplier: "",
            supplierGuess: "",
            supplierQuoteNumber: "",
            issueDate: "",
            validUntil: "",
            currency: "EUR",
            leadTimeWeeks: 0,
            incoterm: "",
            notes: [],
            lines: [],
            warnings: []
        };

        findSupplier(lines, context, result);

        findNumber(lines, context, result);

        findDatesFields(lines, result);

        findCurrency(lines, result);

        findLeadTime(lines, result);

        findIncoterm(lines, result);

        result.lines = parseItems(lines, context, result);

        if (result.lines.length === 0) {
            result.warnings.push(
                "No item lines were recognized - add the materials by hand.");
        }

        return result;
    }

    const api = {
        buildLines: buildLines,
        parseQuotation: parseQuotation,
        parseTierTable: parseTierTable,
        analyzeTierTable: analyzeTierTable,
        parseNumber: parseNumber,
        findDates: findDates
    };

    if (typeof module !== "undefined" && module.exports) {
        module.exports = api;
    }
    else {
        root.QuotationReader = api;
    }

})(typeof window !== "undefined" ? window : this);
