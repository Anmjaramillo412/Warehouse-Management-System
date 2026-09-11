// ============================================================
// MODULE NAVIGATION
// ============================================================

function openModule(module) {

    const dashboard =
        document.querySelector(".dashboard");

    const moduleView =
        document.getElementById("module-view");

    const content =
        document.getElementById("module-content");

    dashboard.style.display = "none";

    moduleView.classList.remove("hidden");

    content.innerHTML = "";


    // ========================================================
    // MATERIAL MANAGER
    // ========================================================

    if (module === "materials") {

        content.innerHTML = `

            <h1>
                Material Manager
            </h1>

            <p>
                Manage materials stored in the system.
            </p>

            <div class="module-buttons">

                <button onclick="showCreateMaterial()">
                    Create Material
                </button>

                <button onclick="showModifyMaterial()">
                    Modify Material
                </button>

                <button onclick="showDeleteMaterial()">
                    Delete Material
                </button>

                <button onclick="showSearchMaterial()">
                    Search Material
                </button>

                <button onclick="displayMaterials()">
                    Display Materials
                </button>

            </div>

            <div id="material-content">
            </div>
        `;
    }

    // ========================================================
    // PRODUCT MANAGER
    // ========================================================

    else if (module === "products") {

        content.innerHTML = `

            <h1>
                Product Manager
            </h1>

            <p>
                Manage products and their Bills of Materials.
            </p>

            <div class="module-buttons">

                <button onclick="showCreateProduct()">
                    Create Product
                </button>

                <button onclick="showDisplayProducts()">
                    Display Products
                </button>

                <button onclick="showDeleteProduct()">
                    Delete Product
                </button>

                <button onclick="showSellProduct()">
                    Sell Product
                </button>

            </div>

            <div id="product-content">
            </div>

        `;
    }

    // ========================================================
    // WAREHOUSE MANAGER
    // ========================================================

    else if (module === "warehouses") {

        content.innerHTML = `

            <h1>
                Warehouse Manager
            </h1>

            <p>
                Manage warehouses and storage locations.
            </p>

            <div class="module-buttons">

                <button onclick="showCreateWarehouse()">
                    Create Warehouse
                </button>

                <button onclick="showDeleteWarehouse()">
                    Delete Warehouse
                </button>

                <button onclick="displayWarehouses()">
                    Display Warehouses
                </button>

            </div>

            <div id="warehouse-content">
            </div>
        `;
    }


    // ========================================================
    // INVENTORY MANAGER
    // ========================================================

    else if (module === "inventory") {

        content.innerHTML = `

            <h1>
                Inventory Manager
            </h1>

            <p>
                Manage material movements and inventory.
            </p>

            <div class="module-buttons">

                <button onclick="showGoodsReceipt()">
                    Goods Receipt
                </button>

                <button onclick="showGoodsIssue()">
                    Goods Issue
                </button>

                <button onclick="showTransferMaterial()">
                    Transfer Material
                </button>

                <button onclick="displayInventory()">
                    Display Inventory
                </button>

                <button onclick="inventoryCheck()">
                    Inventory Check
                </button>

            </div>

            <div id="inventory-content">
            </div>
        `;
    }


    // ========================================================
    // SUPPLIER MANAGER
    // ========================================================

    else if (module === "suppliers") {

        content.innerHTML = `

            <h1>
                Supplier Manager
            </h1>

            <p>
                Manage suppliers referenced by materials.
            </p>

            <div class="module-buttons">

                <button onclick="showCreateSupplier()">
                    Create Supplier
                </button>

                <button onclick="showModifySupplier()">
                    Modify Supplier
                </button>

                <button onclick="displaySuppliers()">
                    Display Suppliers
                </button>

                <button onclick="showDeleteSupplier()">
                    Delete Supplier
                </button>

            </div>

            <div id="supplier-content">
            </div>
        `;
    }

    // ========================================================
    // DATA MANAGEMENT
    // ========================================================

    else if (module === "data") {

        content.innerHTML = `

            <h1>
                Data Management
            </h1>

            <p>
                Save and recover system data.
            </p>


            <div class="module-buttons">

                <button onclick="saveData()">
                    Save Data
                </button>

                <button onclick="loadData()">
                    Load Data
                </button>

                <div class="data-logging-option">

                    <label>

                        <input
                            type="checkbox"
                            id="log-data-operations"
                            onchange="setDataLogging()"
                        >

                        Log Save/Load operations

                    </label>

                </div>

            </div>


            <div id="data-logging-message">
            </div>

        `;


        loadDataLoggingState();
    }
}

// ============================================================
// CREATE SUPPLIER FORM
// ============================================================

function showCreateSupplier() {

    const content =
        document.getElementById("supplier-content");

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Create Supplier
            </h2>

            <label>
                Name
            </label>

            <input
                type="text"
                id="supplier-name"
                placeholder="Enter Supplier Name"
            >


            <label>
                Address
            </label>

            <input
                type="text"
                id="supplier-address"
                placeholder="Enter Address"
            >


            <label>
                Country
            </label>

            <input
                type="text"
                id="supplier-country"
                placeholder="Enter Country"
            >


            <label>
                Contact Name
            </label>

            <input
                type="text"
                id="supplier-contact-name"
                placeholder="Enter Contact Name"
            >


            <label>
                Contact Email
            </label>

            <input
                type="email"
                id="supplier-contact-email"
                placeholder="Enter Contact Email"
            >


            <label>
                Website
            </label>

            <input
                type="text"
                id="supplier-website"
                placeholder="https://..."
            >


            <label>
                Ordering Method
            </label>

            <div class="checkbox-group">

                <label class="checkbox-label">
                    <input type="checkbox" class="supplier-ordering-method" value="Online">
                    Online
                </label>

                <label class="checkbox-label">
                    <input type="checkbox" class="supplier-ordering-method" value="Email">
                    Email
                </label>

                <label class="checkbox-label">
                    <input type="checkbox" class="supplier-ordering-method" value="PO">
                    PO
                </label>

            </div>


            <label>
                Payment Method
            </label>

            <select id="supplier-payment-method">
                <option value="Invoice">Invoice</option>
                <option value="Pre-payment">Pre-payment</option>
                <option value="Online">Online</option>
            </select>


            <label>
                Lead Time (weeks)
            </label>

            <input
                type="number"
                id="supplier-lead-time"
                min="0"
                value="0"
            >


            <div class="form-actions">

                <button onclick="createSupplier()">
                    Create Supplier
                </button>

            </div>


            <div id="supplier-message">
            </div>

        </div>
    `;
}

// ============================================================
// CREATE SUPPLIER
// ============================================================

async function createSupplier() {

    const name =
        document.getElementById(
            "supplier-name"
        ).value.trim();

    const address =
        document.getElementById(
            "supplier-address"
        ).value.trim();

    const country =
        document.getElementById(
            "supplier-country"
        ).value.trim();

    const contactName =
        document.getElementById(
            "supplier-contact-name"
        ).value.trim();

    const contactEmail =
        document.getElementById(
            "supplier-contact-email"
        ).value.trim();

    const website =
        document.getElementById(
            "supplier-website"
        ).value.trim();

    const orderingMethods =
        Array.from(
            document.querySelectorAll(
                ".supplier-ordering-method:checked"
            )
        ).map(checkbox => checkbox.value);

    const paymentMethod =
        document.getElementById(
            "supplier-payment-method"
        ).value;

    const leadTimeWeeks =
        Number(
            document.getElementById(
                "supplier-lead-time"
            ).value
        ) || 0;

    const message =
        document.getElementById(
            "supplier-message"
        );


    if (!name) {

        message.textContent =
            "Supplier name is required.";

        return;
    }


    const supplier = {

        name: name,

        address: address,

        country: country,

        contactName: contactName,

        contactEmail: contactEmail,

        website: website,

        orderingMethods: orderingMethods,

        paymentMethod: paymentMethod,

        leadTimeWeeks: leadTimeWeeks
    };


    try {

        const response =
            await fetch(
                "/api/suppliers/create",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(supplier)
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Supplier created successfully.";

            showCreateSupplier();
        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// MODIFY SUPPLIER FORM
// ============================================================

function showModifySupplier() {

    const content =
        document.getElementById(
            "supplier-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Modify Supplier
            </h2>

            <label>
                Supplier
            </label>

            <select id="ms-lookup-name">
                <option value="">Loading suppliers...</option>
            </select>

            <div class="form-actions">

                <button onclick="loadSupplierForModify()">
                    Load Supplier
                </button>

            </div>

            <div id="modify-supplier-form">
            </div>

        </div>
    `;

    populateSupplierSelect("ms-lookup-name");
}

// ============================================================
// LOAD SUPPLIER FOR MODIFY
// ============================================================

async function loadSupplierForModify() {

    const name =
        document.getElementById(
            "ms-lookup-name"
        ).value;

    const modifyForm =
        document.getElementById(
            "modify-supplier-form"
        );

    if (!name) {

        modifyForm.innerHTML = `
            <p>
                Please select a supplier.
            </p>
        `;

        return;
    }

    modifyForm.innerHTML = `
        <p>
            Loading supplier...
        </p>
    `;

    try {

        const response =
            await fetch(
                `/api/suppliers/${encodeURIComponent(name)}`
            );

        const data =
            await response.json();

        if (!response.ok) {

            modifyForm.innerHTML = `
                <p>
                    ${data.message}
                </p>
            `;

            return;
        }

        const orderingMethods =
            data.orderingMethods || [];

        modifyForm.innerHTML = `

            <input type="hidden" id="ms-name" value="${escapeHtml(data.name)}">

            <p>
                Supplier:
                <strong>${escapeHtml(data.name)}</strong>
            </p>

            <label>
                Address
            </label>

            <input type="text" id="ms-address"
                value="${escapeHtml(data.address || "")}">

            <label>
                Country
            </label>

            <input type="text" id="ms-country"
                value="${escapeHtml(data.country || "")}">

            <label>
                Contact Name
            </label>

            <input type="text" id="ms-contact-name"
                value="${escapeHtml(data.contactName || "")}">

            <label>
                Contact Email
            </label>

            <input type="email" id="ms-contact-email"
                value="${escapeHtml(data.contactEmail || "")}">

            <label>
                Website
            </label>

            <input type="text" id="ms-website"
                value="${escapeHtml(data.website || "")}">

            <label>
                Ordering Method
            </label>

            <div class="checkbox-group">

                <label class="checkbox-label">
                    <input type="checkbox" class="ms-ordering-method" value="Online"
                        ${orderingMethods.includes("Online") ? "checked" : ""}>
                    Online
                </label>

                <label class="checkbox-label">
                    <input type="checkbox" class="ms-ordering-method" value="Email"
                        ${orderingMethods.includes("Email") ? "checked" : ""}>
                    Email
                </label>

                <label class="checkbox-label">
                    <input type="checkbox" class="ms-ordering-method" value="PO"
                        ${orderingMethods.includes("PO") ? "checked" : ""}>
                    PO
                </label>

            </div>

            <label>
                Payment Method
            </label>

            <select id="ms-payment-method">
                <option value="Invoice" ${data.paymentMethod === "Invoice" ? "selected" : ""}>Invoice</option>
                <option value="Pre-payment" ${data.paymentMethod === "Pre-payment" ? "selected" : ""}>Pre-payment</option>
                <option value="Online" ${data.paymentMethod === "Online" ? "selected" : ""}>Online</option>
            </select>

            <label>
                Lead Time (weeks)
            </label>

            <input type="number" id="ms-lead-time" min="0"
                value="${data.leadTimeWeeks || 0}">

            <div class="form-actions">

                <button onclick="modifySupplierSubmit()">
                    Save Changes
                </button>

            </div>

            <div id="modify-supplier-message">
            </div>
        `;
    }
    catch (error) {

        console.error(error);

        modifyForm.innerHTML = `
            <p>
                Could not connect to the server.
            </p>
        `;
    }
}

// ============================================================
// MODIFY SUPPLIER (SUBMIT)
// ============================================================

async function modifySupplierSubmit() {

    const name =
        document.getElementById(
            "ms-name"
        ).value;

    const address =
        document.getElementById(
            "ms-address"
        ).value.trim();

    const country =
        document.getElementById(
            "ms-country"
        ).value.trim();

    const contactName =
        document.getElementById(
            "ms-contact-name"
        ).value.trim();

    const contactEmail =
        document.getElementById(
            "ms-contact-email"
        ).value.trim();

    const website =
        document.getElementById(
            "ms-website"
        ).value.trim();

    const orderingMethods =
        Array.from(
            document.querySelectorAll(
                ".ms-ordering-method:checked"
            )
        ).map(checkbox => checkbox.value);

    const paymentMethod =
        document.getElementById(
            "ms-payment-method"
        ).value;

    const leadTimeWeeks =
        Number(
            document.getElementById(
                "ms-lead-time"
            ).value
        ) || 0;

    const message =
        document.getElementById(
            "modify-supplier-message"
        );

    const supplier = {

        name: name,

        address: address,

        country: country,

        contactName: contactName,

        contactEmail: contactEmail,

        website: website,

        orderingMethods: orderingMethods,

        paymentMethod: paymentMethod,

        leadTimeWeeks: leadTimeWeeks
    };

    try {

        const response =
            await fetch(
                "/api/suppliers/modify",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(supplier)
                }
            );

        const responseText =
            await response.text();

        if (response.ok) {

            message.textContent =
                "Supplier successfully modified.";
        }
        else {

            message.textContent =
                "Error: " + responseText;
        }
    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// DISPLAY SUPPLIERS
// ============================================================

async function displaySuppliers() {

    const content =
        document.getElementById(
            "supplier-content"
        );

    content.innerHTML = `
        <p>Loading suppliers...</p>
    `;


    try {

        const response =
            await fetch("/api/suppliers");


        if (!response.ok) {

            const message =
                await response.text();

            content.innerHTML = `
                <p>
                    Error: ${message}
                </p>
            `;

            return;
        }


        const data =
            await response.json();


        if (
            !data.suppliers ||
            data.suppliers.length === 0
        ) {

            content.innerHTML = `

                <div class="empty-message">

                    No suppliers available.

                </div>
            `;

            return;
        }


        let html = `

            <div class="material-table-container">

                <h2>
                    Suppliers
                </h2>

                <table class="material-table">

                    <thead>

                        <tr>

                            <th>Name</th>
                            <th>Country</th>
                            <th>Contact</th>
                            <th>Email</th>
                            <th>Website</th>
                            <th>Ordering</th>
                            <th>Payment</th>
                            <th>Lead Time</th>

                        </tr>

                    </thead>

                    <tbody>
        `;


        for (const supplier of data.suppliers) {

            html += `

                <tr>

                    <td>${escapeHtml(supplier.name)}</td>
                    <td>${escapeHtml(supplier.country || "")}</td>
                    <td>${escapeHtml(supplier.contactName || "")}</td>
                    <td>${escapeHtml(supplier.contactEmail || "")}</td>
                    <td>${escapeHtml(supplier.website || "")}</td>
                    <td>${escapeHtml((supplier.orderingMethods || []).join(", "))}</td>
                    <td>${escapeHtml(supplier.paymentMethod || "")}</td>
                    <td>${supplier.leadTimeWeeks} week(s)</td>

                </tr>
            `;
        }


        html += `

                    </tbody>

                </table>

            </div>
        `;


        content.innerHTML = html;

    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>
                Could not connect to the server.
            </p>
        `;
    }
}

// ============================================================
// DELETE SUPPLIER FORM
// ============================================================

function showDeleteSupplier() {

    const content =
        document.getElementById(
            "supplier-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Delete Supplier
            </h2>

            <label>
                Supplier
            </label>

            <select id="delete-supplier-name">
                <option value="">Loading suppliers...</option>
            </select>

            <div class="form-actions">

                <button onclick="deleteSupplier()">
                    Delete Supplier
                </button>

            </div>

            <div id="delete-supplier-message">
            </div>

        </div>
    `;

    populateSupplierSelect("delete-supplier-name");
}

// ============================================================
// DELETE SUPPLIER
// ============================================================

async function deleteSupplier() {

    const name =
        document.getElementById(
            "delete-supplier-name"
        ).value;

    const message =
        document.getElementById(
            "delete-supplier-message"
        );


    if (!name) {

        message.textContent =
            "Please select a supplier.";

        return;
    }


    try {

        const response =
            await fetch(
                "/api/suppliers/delete",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify({ name: name })
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Supplier deleted successfully.";

            showDeleteSupplier();
        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// LOAD DATA LOGGING STATE
// ============================================================

async function loadDataLoggingState()
{
    try
    {
        const response =
            await fetch(
                "/api/data/logging"
            );


        const data =
            await response.json();


        if (!response.ok)
        {
            return;
        }


        const checkbox =
            document.getElementById(
                "log-data-operations"
            );


        if (checkbox)
        {
            checkbox.checked =
                data.enabled;
        }
    }
    catch (error)
    {
        console.error(
            "Could not load logging state:",
            error
        );
    }
}

// ============================================================
// SAVE DATA
// ============================================================

async function saveData() {

    const message =
            document.getElementById(
                "data-logging-message"
            );

    try {

        const response =
            await fetch(
                "/api/data/save",
                {
                    method: "POST"
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "✓ Data saved successfully.";

        }
        else {

            message.textContent =
                "Error saving data: "
                + responseText;

        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// LOAD DATA
// ============================================================

async function loadData() {
    
    const message =
        document.getElementById(
            "data-logging-message"
        );

    try {

        const response =
            await fetch(
                "/api/data/load",
                {
                    method: "POST"
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {
            message.textContent =
                "✓ Data loaded successfully.";

        }
        else {

            message.textContent =
                "Error loading data: "
                + responseText;

        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// CREATE MATERIAL FORM
// ============================================================

function showCreateMaterial() {

    const content =
        document.getElementById("material-content");

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Create Material
            </h2>

            <label>
                Material ID
            </label>

            <input
                type="text"
                id="material-id"
                placeholder="###-###### or ######-00"
                maxlength="10"
            >

            <small>
                Format: ###-###### or ######-00
            </small>


            <label>
                Material Name
            </label>

            <input
                type="text"
                id="material-name"
                placeholder="Enter Material Name"
            >


            <label>
                Description
            </label>

            <textarea
                id="material-description"
                placeholder="Enter Material Description"
                rows="4"></textarea>


            <label>
                Unit of Measure
            </label>

            <input
                type="text"
                id="material-uom"
                value="pcs"
                placeholder="pcs"
            >


            <label>
                Category
            </label>

            <input
                type="text"
                id="material-category"
                placeholder="Enter Category"
            >


            <label>
                Material Type
            </label>

            <select
                id="material-type"
                onchange="toggleDrawingNumberField('material')">

                <option value="Standard Part">Standard Part</option>
                <option value="Design Part">Design Part</option>
                <option value="PCB">PCB</option>

            </select>


            <div id="material-drawing-number-group" class="hidden">

                <label>
                    Drawing Number
                </label>

                <input
                    type="text"
                    id="material-drawing-number"
                    placeholder="e.g. 123-ASM-4567"
                >

                <small>
                    Format: ###-ASM-####, ###-PAR-####,
                    I-BU#-##, OWI-BU#-##, or ###-PCB-####
                </small>

            </div>


            <label>
                Manufacturer
            </label>

            <input
                type="text"
                id="material-manufacturer"
                placeholder="Enter Manufacturer"
            >


            <label>
                Manufacturer Part Number
            </label>

            <input
                type="text"
                id="material-manufacturer-pn"
                placeholder="Enter Manufacturer Part Number"
            >


            <label>
                Supplier
            </label>

            <div class="combobox">

                <input
                    type="text"
                    id="material-supplier-search"
                    autocomplete="off"
                    placeholder="Search supplier by name..."
                    oninput="handleSupplierSearchInput('material')"
                    onfocus="renderSupplierOptions('material')"
                    onblur="hideSupplierOptionsDelayed('material')"
                >

                <input type="hidden" id="material-supplier">

                <div id="material-supplier-options"
                    class="combobox-options hidden">
                </div>

            </div>

            <small>
                No supplier listed?
                <a href="#" onclick="openModule('suppliers'); return false;">
                    Create one first.
                </a>
            </small>


            <label>
                Supplier Part Number
            </label>

            <input
                type="text"
                id="material-supplier-pn"
                placeholder="Enter Supplier Part Number"
            >


            <label>
                Material Photo
            </label>

            <input
                type="file"
                id="material-photo"
                accept="image/*"
            >

            <label class="checkbox-label">

                <input
                    type="checkbox"
                    id="material-active"
                    checked
                >

                Active Material

            </label>


            <div class="form-actions">

                <button
                    onclick="createMaterial()">

                    Create Material

                </button>

            </div>


            <div id="material-message">
            </div>

        </div>
    `;

    initSupplierCombobox("material");
}

// ============================================================
// TOGGLE DRAWING NUMBER FIELD
// ============================================================
// Drawing Number only applies to Design Part / PCB materials.

function toggleDrawingNumberField(prefix) {

    const type =
        document.getElementById(
            prefix + "-type"
        ).value;

    const group =
        document.getElementById(
            prefix + "-drawing-number-group"
        );

    const requiresDrawing =
        (type === "Design Part" || type === "PCB");

    group.classList.toggle(
        "hidden",
        !requiresDrawing);
}

// ============================================================
// POPULATE SUPPLIER SELECT
// ============================================================

async function populateSupplierSelect(selectId, selectedName) {

    const select =
        document.getElementById(selectId);

    if (!select) {
        return;
    }

    try {

        const response =
            await fetch("/api/suppliers");

        const data =
            await response.json();

        const suppliers =
            data.suppliers || [];

        if (suppliers.length === 0) {

            select.innerHTML = `
                <option value="">
                    No suppliers available
                </option>
            `;

            return;
        }

        select.innerHTML =
            `<option value="">Select a supplier</option>` +
            suppliers.map(supplier => `
                <option value="${escapeHtml(supplier.name)}">
                    ${escapeHtml(supplier.name)}
                </option>
            `).join("");

        if (selectedName) {
            select.value = selectedName;
        }
    }
    catch (error) {

        console.error(error);

        select.innerHTML = `
            <option value="">
                Could not load suppliers
            </option>
        `;
    }
}

// ============================================================
// SUPPLIER SEARCH COMBOBOX
// ============================================================
// Used by the Material create/modify forms to search for an
// existing supplier by name instead of picking from a long list.
// The hidden "<prefix>-supplier" input only ever holds a name
// that exactly matches an existing supplier - it is cleared
// whenever the visible search text does not match one.

let supplierCache = null;

async function loadSupplierCache(forceRefresh) {

    if (supplierCache && !forceRefresh) {
        return supplierCache;
    }

    try {

        const response =
            await fetch("/api/suppliers");

        const data =
            await response.json();

        supplierCache =
            data.suppliers || [];
    }
    catch (error) {

        console.error(error);

        supplierCache = [];
    }

    return supplierCache;
}

async function initSupplierCombobox(prefix, selectedName) {

    await loadSupplierCache(true);

    const hiddenInput =
        document.getElementById(prefix + "-supplier");

    const searchInput =
        document.getElementById(prefix + "-supplier-search");

    if (!hiddenInput || !searchInput) {
        return;
    }

    hiddenInput.value = selectedName || "";
    searchInput.value = selectedName || "";
}

function renderSupplierOptions(prefix) {

    const searchInput =
        document.getElementById(prefix + "-supplier-search");

    const optionsBox =
        document.getElementById(prefix + "-supplier-options");

    if (!searchInput || !optionsBox) {
        return;
    }

    const filterText =
        searchInput.value.trim().toLowerCase();

    const suppliers =
        supplierCache || [];

    const matches =
        suppliers.filter(supplier =>
            supplier.name.toLowerCase().includes(filterText)
        );

    if (matches.length === 0) {

        optionsBox.innerHTML = `
            <div class="combobox-option combobox-empty">
                No suppliers found
            </div>
        `;
    }
    else {

        optionsBox.innerHTML =
            matches.map(supplier => `
                <div class="combobox-option"
                    onmousedown="selectSupplierOption('${prefix}', '${escapeHtml(supplier.name)}')">
                    ${escapeHtml(supplier.name)}
                </div>
            `).join("");
    }

    optionsBox.classList.remove("hidden");
}

function handleSupplierSearchInput(prefix) {

    renderSupplierOptions(prefix);

    const searchInput =
        document.getElementById(prefix + "-supplier-search");

    const hiddenInput =
        document.getElementById(prefix + "-supplier");

    const suppliers =
        supplierCache || [];

    const typedName =
        searchInput.value.trim();

    const exactMatch =
        suppliers.find(supplier =>
            supplier.name === typedName
        );

    hiddenInput.value =
        exactMatch ? exactMatch.name : "";
}

function selectSupplierOption(prefix, name) {

    document.getElementById(
        prefix + "-supplier"
    ).value = name;

    document.getElementById(
        prefix + "-supplier-search"
    ).value = name;

    document.getElementById(
        prefix + "-supplier-options"
    ).classList.add("hidden");
}

function hideSupplierOptionsDelayed(prefix) {

    // Delay so a click (onmousedown) on an option
    // still registers before the dropdown disappears.

    setTimeout(() => {

        const optionsBox =
            document.getElementById(prefix + "-supplier-options");

        if (optionsBox) {
            optionsBox.classList.add("hidden");
        }

    }, 150);
}

// ============================================================
// CREATE MATERIAL
// ============================================================

async function createMaterial() {

    const id =
        document.getElementById(
            "material-id"
        ).value.trim();

    const name =
        document.getElementById(
            "material-name"
        ).value.trim();

    const description =
        document.getElementById(
            "material-description"
        ).value.trim();

    const uom =
        document.getElementById(
            "material-uom"
        ).value.trim();

    const category =
        document.getElementById(
            "material-category"
        ).value.trim();

    const type =
        document.getElementById(
            "material-type"
        ).value;

    const drawingNumber =
        document.getElementById(
            "material-drawing-number"
        ).value.trim();

    const manufacturer =
        document.getElementById(
            "material-manufacturer"
        ).value.trim();

    const manufacturerPartNumber =
        document.getElementById(
            "material-manufacturer-pn"
        ).value.trim();

    const supplier =
        document.getElementById(
            "material-supplier"
        ).value.trim();

    const supplierPartNumber =
        document.getElementById(
            "material-supplier-pn"
        ).value.trim();

    const active =
        document.getElementById(
            "material-active"
        ).checked;

    const photoInput =
        document.getElementById(
            "material-photo"
        );

    const photoFile =
        photoInput.files[0];

    let photoData = "";
    let photoName = "";

    if (photoFile) {

        photoName =
            photoFile.name;

        photoData =
            await readFileAsBase64(
                photoFile
            );
    }

    // ========================================================
    // VALIDATION
    // ========================================================

    const idPattern =
        /^([0-9]{3}-[0-9]{6}|[0-9]{6}-00)$/;

    const drawingNumberPattern =
        /^([0-9]{3}-ASM-[0-9]{4}|[0-9]{3}-PAR-[0-9]{4}|I-BU[0-9]-[0-9]{2}|OWI-BU[0-9]-[0-9]{2}|[0-9]{3}-PCB-[0-9]{4})$/;

    const message =
        document.getElementById(
            "material-message"
        );


    if (!idPattern.test(id)) {

        message.textContent =
            "Invalid Material ID. Expected format ###-###### or ######-00.";

        return;
    }


    if (!name ||
        !category ||
        !supplier) {

        message.textContent =
            "Please fill in all required fields, including the Supplier.";

        return;
    }


    if ((type === "Design Part" || type === "PCB") &&
        !drawingNumberPattern.test(drawingNumber)) {

        message.textContent =
            "Invalid or missing Drawing Number for this material type.";

        return;
    }


    // ========================================================
    // PREPARE JSON
    // ========================================================

    const material = {

        id: id,

        name: name,

        description: description,

        uom: uom || "pcs",

        category: category,

        type: type,

        drawingNumber: drawingNumber,

        manufacturer: manufacturer,

        manufacturerPartNumber: manufacturerPartNumber,

        supplier: supplier,

        supplierPartNumber: supplierPartNumber,

        photo: photoName,

        photoData: photoData,

        active: active
    };


    // ========================================================
    // SEND TO C++
    // ========================================================

    try {

        const response =
            await fetch(
                "/api/materials/create",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(material)
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Material created successfully.";


            document.getElementById(
                "material-id"
            ).value = "";

            document.getElementById(
                "material-name"
            ).value = "";

            document.getElementById(
                "material-description"
            ).value = "";

            document.getElementById(
                "material-uom"
            ).value = "pcs";

            document.getElementById(
                "material-category"
            ).value = "";

            document.getElementById(
                "material-type"
            ).value = "Standard Part";

            document.getElementById(
                "material-drawing-number"
            ).value = "";

            toggleDrawingNumberField("material");

            document.getElementById(
                "material-manufacturer"
            ).value = "";

            document.getElementById(
                "material-manufacturer-pn"
            ).value = "";

            document.getElementById(
                "material-supplier"
            ).value = "";

            document.getElementById(
                "material-supplier-search"
            ).value = "";

            document.getElementById(
                "material-supplier-pn"
            ).value = "";

            document.getElementById(
                "material-photo"
            ).value = "";

            document.getElementById(
                "material-active"
            ).checked = true;

        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// DISPLAY MATERIALS
// ============================================================

async function displayMaterials() {

    const content =
        document.getElementById(
            "material-content"
        );

    content.innerHTML = `
        <p>Loading materials...</p>
    `;


    try {

        const response =
            await fetch(
                "/api/materials"
            );


        if (!response.ok) {

            const message =
                await response.text();

            content.innerHTML = `
                <p>
                    Error: ${message}
                </p>
            `;

            return;
        }


        const data =
            await response.json();


        if (
            !data.materials ||
            data.materials.length === 0
        ) {

            content.innerHTML = `

                <div class="empty-message">

                    No materials available.

                </div>
            `;

            return;
        }


        let html = `

            <div class="material-table-container">

                <h2>
                    Materials
                </h2>

                <table class="material-table material-table-clickable">

                    <thead>

                        <tr>

                            <th>Photo</th>
                            <th>Material ID</th>
                            <th>Name</th>
                            <th>UoM</th>
                            <th>Drawing Number</th>
                            <th>Manufacturer</th>
                            <th>Mfr Part #</th>
                            <th>Supplier</th>
                            <th>Supplier Part #</th>

                        </tr>

                    </thead>

                    <tbody>
        `;


        for (
            const material of data.materials
        ) {

            html += `

                <tr
                    onclick="showMaterialDetail('${escapeHtml(material.id)}')"
                    title="Click to view full material details">

                    <td>

                        ${
                            material.photo
                            ? `<img
                                    src="/${material.photo}"
                                    class="material-thumbnail"
                                    alt="Material photo"
                               >`
                            : "No photo"
                        }

                    </td>

                    <td>
                        ${escapeHtml(material.id)}
                    </td>

                    <td>
                        ${escapeHtml(material.name)}
                    </td>

                    <td>
                        ${escapeHtml(material.uom || "")}
                    </td>

                    <td>
                        ${escapeHtml(material.drawingNumber || "")}
                    </td>

                    <td>
                        ${escapeHtml(material.manufacturer || "")}
                    </td>

                    <td>
                        ${escapeHtml(material.manufacturerPartNumber || "")}
                    </td>

                    <td>
                        ${escapeHtml(material.supplier || "")}
                    </td>

                    <td>
                        ${escapeHtml(material.supplierPartNumber || "")}
                    </td>

                </tr>
            `;
        }


        html += `

                    </tbody>

                </table>

            </div>
        `;


        content.innerHTML = html;

    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>
                Could not connect to the server.
            </p>
        `;
    }
}

// ============================================================
// MATERIAL DETAIL DASHBOARD
// ============================================================

async function showMaterialDetail(id) {

    const content =
        document.getElementById(
            "material-content"
        );

    content.innerHTML = `
        <p>Loading material...</p>
    `;


    try {

        const response =
            await fetch(
                `/api/materials/${encodeURIComponent(id)}`
            );


        const data =
            await response.json();


        if (!response.ok) {

            content.innerHTML = `
                <p>
                    ${escapeHtml(data.message || "Material not found.")}
                </p>

                <div class="module-buttons">
                    <button onclick="displayMaterials()">
                        ← Back to Materials
                    </button>
                </div>
            `;

            return;
        }


        content.innerHTML = `

            <div class="material-detail">

                <div class="module-buttons material-detail-back">
                    <button onclick="displayMaterials()">
                        ← Back to Materials
                    </button>
                </div>

                <div class="material-detail-header">

                    <div class="material-detail-photo">
                        ${
                            data.photo
                            ? `<img
                                    src="/${data.photo}"
                                    alt="Material photo"
                               >`
                            : `<div class="material-detail-photo-placeholder">
                                    No photo
                               </div>`
                        }
                    </div>

                    <div class="material-detail-title">

                        <h2>
                            ${escapeHtml(data.name)}
                        </h2>

                        <p class="material-detail-id">
                            ${escapeHtml(data.id)}
                        </p>

                        <span class="status-badge ${data.active ? "status-active" : "status-inactive"}">
                            ${data.active ? "Active" : "Inactive"}
                        </span>

                    </div>

                </div>

                <div class="material-detail-grid">

                    <div class="detail-field">
                        <div class="detail-label">Description</div>
                        <div class="detail-value">${escapeHtml(data.description || "—")}</div>
                    </div>

                    <div class="detail-field">
                        <div class="detail-label">Unit of Measure</div>
                        <div class="detail-value">${escapeHtml(data.uom || "—")}</div>
                    </div>

                    <div class="detail-field">
                        <div class="detail-label">Category</div>
                        <div class="detail-value">${escapeHtml(data.category || "—")}</div>
                    </div>

                    <div class="detail-field">
                        <div class="detail-label">Type</div>
                        <div class="detail-value">${escapeHtml(data.type || "—")}</div>
                    </div>

                    <div class="detail-field">
                        <div class="detail-label">Drawing Number</div>
                        <div class="detail-value">${escapeHtml(data.drawingNumber || "—")}</div>
                    </div>

                    <div class="detail-field">
                        <div class="detail-label">Manufacturer</div>
                        <div class="detail-value">${escapeHtml(data.manufacturer || "—")}</div>
                    </div>

                    <div class="detail-field">
                        <div class="detail-label">Manufacturer Part Number</div>
                        <div class="detail-value">${escapeHtml(data.manufacturerPartNumber || "—")}</div>
                    </div>

                    <div class="detail-field">
                        <div class="detail-label">Supplier</div>
                        <div class="detail-value">${escapeHtml(data.supplier || "—")}</div>
                    </div>

                    <div class="detail-field">
                        <div class="detail-label">Supplier Part Number</div>
                        <div class="detail-value">${escapeHtml(data.supplierPartNumber || "—")}</div>
                    </div>

                </div>

            </div>
        `;

    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>
                Could not connect to the server.
            </p>
        `;
    }
}

// ============================================================
// SEARCH MATERIAL FORM
// ============================================================

function showSearchMaterial() {

    const content =
        document.getElementById(
            "material-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Search Material
            </h2>

            <label>
                Material ID
            </label>

            <input
                type="text"
                id="search-material-id"
                placeholder="###-######"
                maxlength="10"
            >

            <div class="form-actions">

                <button
                    onclick="searchMaterial()">

                    Search Material

                </button>

            </div>

            <div id="search-material-result">
            </div>

        </div>
    `;
}


// ============================================================
// SEARCH MATERIAL
// ============================================================

async function searchMaterial() {

    const id =
        document.getElementById(
            "search-material-id"
        ).value.trim();


    const result =
        document.getElementById(
            "search-material-result"
        );


    const idPattern =
        /^([0-9]{3}-[0-9]{6}|[0-9]{6}-00)$/;


    if (!idPattern.test(id)) {

        result.innerHTML = `
            <p>
                Invalid Material ID.
                Expected format ###-###### or ######-00.
            </p>
        `;

        return;
    }


    result.innerHTML = `
        <p>
            Searching...
        </p>
    `;


    try {

        const response =
            await fetch(
                `/api/materials/${encodeURIComponent(id)}`
            );


        const data =
            await response.json();


        if (!response.ok) {

            result.innerHTML = `
                <p>
                    ${data.message}
                </p>
            `;

            return;
        }


        result.innerHTML = `

            <div class="search-result">

                <h3>
                    Material Found
                </h3>

                <p>
                    <strong>ID:</strong>
                    ${data.id}
                </p>

                <p>
                    <strong>Name:</strong>
                    ${data.name}
                </p>

                <p>
                    <strong>Description:</strong>
                    ${data.description}
                </p>

                <p>
                    <strong>Unit of Measure:</strong>
                    ${data.uom}
                </p>

                <p>
                    <strong>Category:</strong>
                    ${data.category}
                </p>

                <p>
                    <strong>Type:</strong>
                    ${data.type}
                </p>

                ${
                    data.drawingNumber
                    ? `<p>
                        <strong>Drawing Number:</strong>
                        ${data.drawingNumber}
                    </p>`
                    : ""
                }

                <p>
                    <strong>Manufacturer:</strong>
                    ${data.manufacturer || ""}
                </p>

                <p>
                    <strong>Manufacturer Part Number:</strong>
                    ${data.manufacturerPartNumber || ""}
                </p>

                <p>
                    <strong>Supplier:</strong>
                    ${data.supplier}
                </p>

                <p>
                    <strong>Supplier Part Number:</strong>
                    ${data.supplierPartNumber || ""}
                </p>

                <p>
                    <strong>Active:</strong>
                    ${data.active ? "Yes" : "No"}
                </p>

            </div>
        `;

    }
    catch (error) {

        console.error(error);

        result.innerHTML = `
            <p>
                Could not connect to the server.
            </p>
        `;
    }
}


// ============================================================
// MODIFY MATERIAL FORM
// ============================================================

function showModifyMaterial() {

    const content =
        document.getElementById(
            "material-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Modify Material
            </h2>

            <label>
                Material ID
            </label>

            <input
                type="text"
                id="modify-material-id"
                placeholder="###-######"
                maxlength="10"
            >

            <div class="form-actions">

                <button
                    onclick="loadMaterialForModify()">

                    Load Material

                </button>

            </div>

            <div id="modify-form">
            </div>

        </div>
    `;
}

// ============================================================
// DELETE MATERIAL FORM
// ============================================================

function showDeleteMaterial() {

    const content =
        document.getElementById(
            "material-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Delete Material
            </h2>


            <label>
                Material ID
            </label>

            <input
                type="text"
                id="delete-material-id"
                placeholder="###-######"
                maxlength="10"
            >


            <div class="form-actions">

                <button
                    onclick="loadMaterialForDelete()">

                    Find Material

                </button>

            </div>


            <div id="delete-content">
            </div>

        </div>
    `;
}

// ============================================================
// LOAD MATERIAL FOR DELETE
// ============================================================

async function loadMaterialForDelete() {

    const id =
        document.getElementById(
            "delete-material-id"
        ).value.trim();


    const content =
        document.getElementById(
            "delete-content"
        );


    const idPattern =
        /^([0-9]{3}-[0-9]{6}|[0-9]{6}-00)$/;


    if (!idPattern.test(id)) {

        content.innerHTML = `
            <p>
                Invalid Material ID.
                Expected format ###-###### or ######-00.
            </p>
        `;

        return;
    }


    content.innerHTML = `
        <p>
            Loading material...
        </p>
    `;


    try {

        const response =
            await fetch(
                `/api/materials/${encodeURIComponent(id)}`
            );


        const data =
            await response.json();


        if (!response.ok) {

            content.innerHTML = `
                <p>
                    ${data.message}
                </p>
            `;

            return;
        }


        content.innerHTML = `

            <div class="search-result">

                <h3>
                    Material to Delete
                </h3>

                <p>
                    <strong>ID:</strong>
                    ${data.id}
                </p>

                <p>
                    <strong>Name:</strong>
                    ${escapeHtml(data.name)}
                </p>

                <p>
                    <strong>Description:</strong>
                    ${escapeHtml(data.description || "")}
                </p>

                <p>
                    <strong>Category:</strong>
                    ${escapeHtml(data.category)}
                </p>

                <p>
                    <strong>Supplier:</strong>
                    ${escapeHtml(data.supplier)}
                </p>


                <div class="delete-confirmation">

                    <p>
                        <strong>
                            Are you sure you want to delete
                            this material?
                        </strong>
                    </p>


                    <button
                        class="delete-button"
                        onclick="deleteMaterial(
                            '${data.id}'
                        )">

                        Yes, Delete Material

                    </button>


                    <button
                        class="cancel-button"
                        onclick="showDeleteMaterial()">

                        Cancel

                    </button>

                </div>

            </div>

        `;
    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>
                Could not connect to the server.
            </p>
        `;
    }
}

// ============================================================
// DELETE MATERIAL
// ============================================================

async function deleteMaterial(id) {

    const content =
        document.getElementById(
            "material-content"
        );


    try {

        const response =
            await fetch(
                "/api/materials/delete",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify({
                            id: id
                        })
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            content.innerHTML = `

                <div class="search-result">

                    <h3>
                        Material Deleted
                    </h3>

                    <p>
                        Material
                        <strong>${escapeHtml(id)}</strong>
                        was deleted successfully.
                    </p>

                </div>
            `;
        }
        else {

            content.innerHTML = `

                <div class="search-result">

                    <p>
                        Error: ${escapeHtml(responseText)}
                    </p>

                </div>
            `;
        }
    }
    catch (error) {

        console.error(error);

        content.innerHTML = `

            <p>
                Could not connect to the server.
            </p>

        `;
    }
}

// ============================================================
// LOAD MATERIAL FOR MODIFY
// ============================================================

async function loadMaterialForModify() {

    const id =
        document.getElementById(
            "modify-material-id"
        ).value.trim();


    const modifyForm =
        document.getElementById(
            "modify-form"
        );


    const idPattern =
        /^([0-9]{3}-[0-9]{6}|[0-9]{6}-00)$/;


    if (!idPattern.test(id)) {

        modifyForm.innerHTML = `
            <p>
                Invalid Material ID.
                Expected format ###-###### or ######-00.
            </p>
        `;

        return;
    }


    modifyForm.innerHTML = `
        <p>
            Loading material...
        </p>
    `;


    try {

        const response =
            await fetch(
                `/api/materials/${encodeURIComponent(id)}`
            );


        const data =
            await response.json();


        if (!response.ok) {

            modifyForm.innerHTML = `
                <p>
                    ${data.message}
                </p>
            `;

            return;
        }


        modifyForm.innerHTML = `

            <div class="form-container">

                <h2>
                    Modify Material
                </h2>


                <p>
                    Material ID:
                    <strong>${data.id}</strong>
                </p>


                <label>
                    Material Name
                </label>

                <input
                    type="text"
                    id="modify-name"
                    value="${escapeHtml(data.name)}"
                >


                <label>
                    Description
                </label>

                <textarea
                    id="modify-description"
                    rows="4"
                >${escapeHtml(data.description || "")}</textarea>


                <label>
                    Unit of Measure
                </label>

                <input
                    type="text"
                    id="modify-uom"
                    value="${escapeHtml(data.uom)}"
                >


                <label>
                    Category
                </label>

                <input
                    type="text"
                    id="modify-category"
                    value="${escapeHtml(data.category)}"
                >


                <label>
                    Material Type
                </label>

                <select
                    id="modify-type"
                    onchange="toggleDrawingNumberField('modify')">

                    <option value="Standard Part"
                        ${data.type === "Standard Part" ? "selected" : ""}>
                        Standard Part
                    </option>

                    <option value="Design Part"
                        ${data.type === "Design Part" ? "selected" : ""}>
                        Design Part
                    </option>

                    <option value="PCB"
                        ${data.type === "PCB" ? "selected" : ""}>
                        PCB
                    </option>

                </select>


                <div id="modify-drawing-number-group"
                    class="${
                        (data.type === "Design Part" || data.type === "PCB")
                        ? ""
                        : "hidden"
                    }">

                    <label>
                        Drawing Number
                    </label>

                    <input
                        type="text"
                        id="modify-drawing-number"
                        value="${escapeHtml(data.drawingNumber || "")}"
                        placeholder="e.g. 123-ASM-4567"
                    >

                    <small>
                        Format: ###-ASM-####, ###-PAR-####,
                        I-BU#-##, OWI-BU#-##, or ###-PCB-####
                    </small>

                </div>


                <label>
                    Manufacturer
                </label>

                <input
                    type="text"
                    id="modify-manufacturer"
                    value="${escapeHtml(data.manufacturer || "")}"
                >


                <label>
                    Manufacturer Part Number
                </label>

                <input
                    type="text"
                    id="modify-manufacturer-pn"
                    value="${escapeHtml(data.manufacturerPartNumber || "")}"
                >


                <label>
                    Supplier
                </label>

                <div class="combobox">

                    <input
                        type="text"
                        id="modify-supplier-search"
                        autocomplete="off"
                        placeholder="Search supplier by name..."
                        oninput="handleSupplierSearchInput('modify')"
                        onfocus="renderSupplierOptions('modify')"
                        onblur="hideSupplierOptionsDelayed('modify')"
                    >

                    <input type="hidden" id="modify-supplier">

                    <div id="modify-supplier-options"
                        class="combobox-options hidden">
                    </div>

                </div>


                <label>
                    Supplier Part Number
                </label>

                <input
                    type="text"
                    id="modify-supplier-pn"
                    value="${escapeHtml(data.supplierPartNumber || "")}"
                >

                <label>
                    Material Photo
                </label>

                <input
                    type="file"
                    id="modify-photo-input"
                    accept="image/*"
                >

                <input
                    type="hidden"
                    id="modify-photo"
                    value="${escapeHtml(data.photo || "")}"
                >

                <label class="checkbox-label">

                    <input
                        type="checkbox"
                        id="modify-active"
                        ${data.active ? "checked" : ""}
                    >

                    Active Material

                </label>


                <div class="form-actions">

                    <button
                        onclick="modifyMaterial()">

                        Save Changes

                    </button>

                </div>


                <div id="modify-message">
                </div>

            </div>

        `;

        initSupplierCombobox(
            "modify",
            data.supplier);
    }
    catch (error) {

        console.error(error);

        modifyForm.innerHTML = `
            <p>
                Could not connect to the server.
            </p>
        `;
    }
}

// ============================================================
// HTML ESCAPE
// ============================================================

function escapeHtml(value) {

    return String(value)
        .replace(/&/g, "&amp;")
        .replace(/</g, "&lt;")
        .replace(/>/g, "&gt;")
        .replace(/"/g, "&quot;")
        .replace(/'/g, "&#039;");
}

// ============================================================
// MODIFY MATERIAL
// ============================================================

async function modifyMaterial() {

    const id =
        document.getElementById(
            "modify-material-id"
        ).value.trim();

    const name =
        document.getElementById(
            "modify-name"
        ).value.trim();

    const description =
        document.getElementById(
            "modify-description"
        ).value.trim();

    const uom =
        document.getElementById(
            "modify-uom"
        ).value.trim();

    const category =
        document.getElementById(
            "modify-category"
        ).value.trim();

    const type =
        document.getElementById(
            "modify-type"
        ).value;

    const drawingNumber =
        document.getElementById(
            "modify-drawing-number"
        ).value.trim();

    const manufacturer =
        document.getElementById(
            "modify-manufacturer"
        ).value.trim();

    const manufacturerPartNumber =
        document.getElementById(
            "modify-manufacturer-pn"
        ).value.trim();

    const supplier =
        document.getElementById(
            "modify-supplier"
        ).value.trim();

    const supplierPartNumber =
        document.getElementById(
            "modify-supplier-pn"
        ).value.trim();

    const active =
        document.getElementById(
            "modify-active"
        ).checked;

    const photoInput =
        document.getElementById(
            "modify-photo-input"
        );

    const photoFile =
        photoInput.files[0];

    let photoPath =
        document.getElementById(
            "modify-photo"
        ).value;


    let photoData = "";


    if (photoFile)
    {
        photoData =
            await readFileAsBase64(
                photoFile
            );
    }

    const message =
        document.getElementById(
            "modify-message"
        );

    const drawingNumberPattern =
        /^([0-9]{3}-ASM-[0-9]{4}|[0-9]{3}-PAR-[0-9]{4}|I-BU[0-9]-[0-9]{2}|OWI-BU[0-9]-[0-9]{2}|[0-9]{3}-PCB-[0-9]{4})$/;

    if (!name ||
        !category ||
        !supplier) {

        message.textContent =
            "Please fill in all required fields, including the Supplier.";

        return;
    }


    if ((type === "Design Part" || type === "PCB") &&
        !drawingNumberPattern.test(drawingNumber)) {

        message.textContent =
            "Invalid or missing Drawing Number for this material type.";

        return;
    }


    const material = {

        id: id,

        name: name,

        description: description,

        uom: uom || "pcs",

        category: category,

        type: type,

        drawingNumber: drawingNumber,

        manufacturer: manufacturer,

        manufacturerPartNumber: manufacturerPartNumber,

        supplier: supplier,

        supplierPartNumber: supplierPartNumber,

        photo: photoPath,

        photoData: photoData,

        active: active
    };


    try {

        const response =
            await fetch(
                "/api/materials/modify",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(material)
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Material successfully modified.";

        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// CREATE WAREHOUSE FORM
// ============================================================

function showCreateWarehouse() {

    const content =
        document.getElementById(
            "warehouse-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Create Warehouse
            </h2>


            <label>
                Warehouse ID
            </label>

            <input
                type="number"
                id="warehouse-id"
                placeholder="Enter Warehouse ID"
            >


            <label>
                Warehouse Name
            </label>

            <input
                type="text"
                id="warehouse-name"
                placeholder="Enter Warehouse Name"
            >


            <div class="form-actions">

                <button
                    onclick="createWarehouse()">

                    Create Warehouse

                </button>

            </div>


            <div id="warehouse-message">
            </div>

        </div>
    `;
}

// ============================================================
// CREATE WAREHOUSE
// ============================================================

async function createWarehouse() {

    const id =
        document.getElementById(
            "warehouse-id"
        ).value;

    const name =
        document.getElementById(
            "warehouse-name"
        ).value.trim();


    const message =
        document.getElementById(
            "warehouse-message"
        );


    if (!id || !name) {

        message.textContent =
            "Please fill in all required fields.";

        return;
    }


    const warehouse = {

        id: Number(id),

        name: name

    };


    try {

        const response =
            await fetch(
                "/api/warehouses/create",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(warehouse)
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Warehouse created successfully.";


            document.getElementById(
                "warehouse-id"
            ).value = "";

            document.getElementById(
                "warehouse-name"
            ).value = "";

        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// DISPLAY WAREHOUSES
// ============================================================

async function displayWarehouses() {

    const content =
        document.getElementById(
            "warehouse-content"
        );


    content.innerHTML = `
        <p>
            Loading warehouses...
        </p>
    `;


    try {

        const response =
            await fetch(
                "/api/warehouses"
            );


        if (!response.ok) {

            const message =
                await response.text();

            content.innerHTML = `
                <p>
                    Error: ${message}
                </p>
            `;

            return;
        }


        const data =
            await response.json();


        if (
            !data.warehouses ||
            data.warehouses.length === 0
        ) {

            content.innerHTML = `
                <div class="empty-message">
                    No warehouses available.
                </div>
            `;

            return;
        }


        let html = `

            <div class="material-table-container">

                <h2>
                    Warehouses
                </h2>

                <table class="material-table">

                    <thead>

                        <tr>

                            <th>Warehouse ID</th>
                            <th>Warehouse Name</th>

                        </tr>

                    </thead>

                    <tbody>
        `;


        for (
            const warehouse of data.warehouses
        ) {

            html += `

                <tr>

                    <td>
                        ${warehouse.id}
                    </td>

                    <td>
                        ${escapeHtml(
                            warehouse.name
                        )}
                    </td>

                </tr>
            `;
        }


        html += `

                    </tbody>

                </table>

            </div>
        `;


        content.innerHTML = html;

    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>
                Could not connect to the server.
            </p>
        `;
    }
}

// ============================================================
// DELETE WAREHOUSE FORM
// ============================================================

function showDeleteWarehouse() {

    const content =
        document.getElementById(
            "warehouse-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Delete Warehouse
            </h2>


            <label>
                Warehouse ID
            </label>

            <input
                type="number"
                id="delete-warehouse-id"
                placeholder="Enter Warehouse ID"
            >


            <div class="form-actions">

                <button
                    onclick="deleteWarehouse()">

                    Delete Warehouse

                </button>

            </div>


            <div id="warehouse-delete-message">
            </div>

        </div>
    `;
}

// ============================================================
// DELETE WAREHOUSE
// ============================================================

async function deleteWarehouse() {

    const id =
        document.getElementById(
            "delete-warehouse-id"
        ).value;


    const message =
        document.getElementById(
            "warehouse-delete-message"
        );


    if (!id) {

        message.textContent =
            "Please enter a Warehouse ID.";

        return;
    }


    try {

        const response =
            await fetch(
                "/api/warehouses/delete",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify({
                            id: Number(id)
                        })
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Warehouse deleted successfully.";


            document.getElementById(
                "delete-warehouse-id"
            ).value = "";

        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// GOODS RECEIPT FORM
// ============================================================

function showGoodsReceipt() {

    const content =
        document.getElementById(
            "inventory-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Goods Receipt
            </h2>


            <label>
                Warehouse ID
            </label>

            <input
                type="number"
                id="receipt-warehouse-id"
                placeholder="Enter Warehouse ID"
            >


            <label>
                Material ID
            </label>

            <input
                type="text"
                id="receipt-material-id"
                placeholder="###-######"
                maxlength="10"
            >


            <label>
                Quantity
            </label>

            <input
                type="number"
                id="receipt-quantity"
                min="1"
                placeholder="Enter Quantity"
            >

            <label>
                Comment
            </label>

            <textarea
                id="receipt-comment"
                rows="4"
                placeholder="Reason or additional information"></textarea>


            <div class="form-actions">

                <button
                    onclick="goodsReceipt()">

                    Receive Goods

                </button>

            </div>


            <div id="receipt-message">
            </div>

        </div>
    `;
}

// ============================================================
// GOODS RECEIPT
// ============================================================

async function goodsReceipt() {

    const warehouseID =
        document.getElementById(
            "receipt-warehouse-id"
        ).value;

    const materialID =
        document.getElementById(
            "receipt-material-id"
        ).value.trim();

    const quantity =
        document.getElementById(
            "receipt-quantity"
        ).value;

    const comment =
        document.getElementById(
            "receipt-comment"
        ).value.trim();

    const message =
        document.getElementById(
            "receipt-message"
        );


    // --------------------------------------------------------
    // Validation
    // --------------------------------------------------------

    const materialIDPattern =
        /^([0-9]{3}-[0-9]{6}|[0-9]{6}-00)$/;


    if (!warehouseID) {

        message.textContent =
            "Please enter a Warehouse ID.";

        return;
    }


    if (!materialIDPattern.test(materialID)) {

        message.textContent =
            "Invalid Material ID. Expected format ###-###### or ######-00.";

        return;
    }


    if (!quantity ||
        Number(quantity) <= 0) {

        message.textContent =
            "Quantity must be greater than zero.";

        return;
    }


    // --------------------------------------------------------
    // Prepare request
    // --------------------------------------------------------

    const receipt = {

        warehouseID:
            Number(warehouseID),

        materialID:
            materialID,

        quantity:
            Number(quantity),

        comment:
            comment

    };


    // --------------------------------------------------------
    // Send to C++
    // --------------------------------------------------------

    try {

        const response =
            await fetch(
                "/api/inventory/receipt",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(receipt)
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Goods receipt completed successfully.";

            clearInputFields([
                "receipt-warehouse-id",
                "receipt-material-id",
                "receipt-quantity",
                "receipt-comment"
            ]);

        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// GOODS ISSUE FORM
// ============================================================

function showGoodsIssue() {

    const content =
        document.getElementById(
            "inventory-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Goods Issue
            </h2>


            <label>
                Warehouse ID
            </label>

            <input
                type="number"
                id="issue-warehouse-id"
                placeholder="Enter Warehouse ID"
            >


            <label>
                Material ID
            </label>

            <input
                type="text"
                id="issue-material-id"
                placeholder="###-######"
                maxlength="10"
            >


            <label>
                Quantity
            </label>

            <input
                type="number"
                id="issue-quantity"
                min="1"
                placeholder="Enter Quantity"
            >

            <label>
                Comment
            </label>

            <textarea
                id="issue-comment"
                rows="4"
                placeholder="Reason or additional information"></textarea>


            <div class="form-actions">

                <button
                    onclick="goodsIssue()">

                    Issue Goods

                </button>

            </div>


            <div id="issue-message">
            </div>

        </div>
    `;
}

// ============================================================
// GOODS ISSUE
// ============================================================

async function goodsIssue() {

    const warehouseID =
        document.getElementById(
            "issue-warehouse-id"
        ).value;

    const materialID =
        document.getElementById(
            "issue-material-id"
        ).value.trim();

    const quantity =
        document.getElementById(
            "issue-quantity"
        ).value;

    const comment =
        document.getElementById(
            "issue-comment"
        ).value.trim();


    const message =
        document.getElementById(
            "issue-message"
        );


    // --------------------------------------------------------
    // Validation
    // --------------------------------------------------------

    const materialIDPattern =
        /^([0-9]{3}-[0-9]{6}|[0-9]{6}-00)$/;


    if (!warehouseID) {

        message.textContent =
            "Please enter a Warehouse ID.";

        return;
    }


    if (!materialIDPattern.test(materialID)) {

        message.textContent =
            "Invalid Material ID. Expected format ###-###### or ######-00.";

        return;
    }


    if (!quantity ||
        Number(quantity) <= 0) {

        message.textContent =
            "Quantity must be greater than zero.";

        return;
    }


    // --------------------------------------------------------
    // Prepare request
    // --------------------------------------------------------

    const issue = {

        warehouseID:
            Number(warehouseID),

        materialID:
            materialID,

        quantity:
            Number(quantity),

        comment:
            comment
    };


    // --------------------------------------------------------
    // Send request
    // --------------------------------------------------------

    try {

        const response =
            await fetch(
                "/api/inventory/issue",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(issue)
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Goods issue completed successfully.";

            clearInputFields([
                "issue-warehouse-id",
                "issue-material-id",
                "issue-quantity",
                "issue-comment"
            ]);

        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// TRANSFER MATERIAL FORM
// ============================================================

function showTransferMaterial() {

    const content =
        document.getElementById(
            "inventory-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Transfer Material
            </h2>


            <label>
                Source Warehouse ID
            </label>

            <input
                type="number"
                id="transfer-source"
                placeholder="Enter Source Warehouse ID"
            >


            <label>
                Destination Warehouse ID
            </label>

            <input
                type="number"
                id="transfer-destination"
                placeholder="Enter Destination Warehouse ID"
            >


            <label>
                Material ID
            </label>

            <input
                type="text"
                id="transfer-material-id"
                placeholder="###-######"
                maxlength="10"
            >


            <label>
                Quantity
            </label>

            <input
                type="number"
                id="transfer-quantity"
                min="1"
                placeholder="Enter Quantity"
            >

            <label>
                Comment
            </label>

            <textarea
                id="transfer-comment"
                rows="4"
                placeholder="Reason or additional information"></textarea>

            <div class="form-actions">

                <button
                    onclick="transferMaterial()">

                    Transfer Material

                </button>

            </div>


            <div id="transfer-message">
            </div>

        </div>
    `;
}

// ============================================================
// TRANSFER MATERIAL
// ============================================================

async function transferMaterial() {

    const sourceWarehouseID =
        document.getElementById(
            "transfer-source"
        ).value;

    const destinationWarehouseID =
        document.getElementById(
            "transfer-destination"
        ).value;

    const materialID =
        document.getElementById(
            "transfer-material-id"
        ).value.trim();

    const quantity =
        document.getElementById(
            "transfer-quantity"
        ).value;

    const comment =
        document.getElementById(
            "transfer-comment"
        ).value.trim();

    const message =
        document.getElementById(
            "transfer-message"
        );

    const materialIDPattern =
        /^([0-9]{3}-[0-9]{6}|[0-9]{6}-00)$/;


    // --------------------------------------------------------
    // Validation
    // --------------------------------------------------------

    if (!sourceWarehouseID ||
        !destinationWarehouseID) {

        message.textContent =
            "Please enter both warehouse IDs.";

        return;
    }


    if (
        Number(sourceWarehouseID) ===
        Number(destinationWarehouseID)
    ) {

        message.textContent =
            "Source and destination warehouses must be different.";

        return;
    }


    if (!materialIDPattern.test(materialID)) {

        message.textContent =
            "Invalid Material ID. Expected format ###-###### or ######-00.";

        return;
    }


    if (!quantity ||
        Number(quantity) <= 0) {

        message.textContent =
            "Quantity must be greater than zero.";

        return;
    }


    const transfer = {

        sourceWarehouseID:
            Number(sourceWarehouseID),

        destinationWarehouseID:
            Number(destinationWarehouseID),

        materialID:
            materialID,

        quantity:
            Number(quantity),

        comment:
            comment
    };

    // --------------------------------------------------------
    // Send to C++
    // --------------------------------------------------------

    try {

        const response =
            await fetch(
                "/api/inventory/transfer",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(transfer)
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Material transferred successfully.";

            clearInputFields([
                "transfer-source",
                "transfer-destination",
                "transfer-material-id",
                "transfer-quantity",
                "transfer-comment"
            ]);
        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// DISPLAY INVENTORY
// ============================================================

async function displayInventory() {

    const content =
        document.getElementById(
            "inventory-content"
        );


    content.innerHTML = `
        <p>
            Loading inventory...
        </p>
    `;


    try {

        const response =
            await fetch(
                "/api/inventory"
            );


        if (!response.ok) {

            const message =
                await response.text();

            content.innerHTML = `
                <p>
                    Error: ${message}
                </p>
            `;

            return;
        }


        const data =
            await response.json();


        if (
            !data.warehouses ||
            data.warehouses.length === 0
        ) {

            content.innerHTML = `
                <div class="empty-message">

                    No inventory available.

                </div>
            `;

            return;
        }


        let html = "";


        for (
            const warehouse of data.warehouses
        ) {

            html += `

                <div class="material-table-container">

                    <h2>
                        Warehouse
                        ${warehouse.id}
                        -
                        ${escapeHtml(
                            warehouse.name
                        )}
                    </h2>
            `;


            if (
                !warehouse.inventory ||
                warehouse.inventory.length === 0
            ) {

                html += `

                    <div class="empty-message">

                        Warehouse is empty.

                    </div>

                `;

            }
            else {

                html += `

                    <table class="material-table">

                        <thead>

                            <tr>

                                <th>
                                    Material ID
                                </th>

                                <th>
                                    Material Name
                                </th>

                                <th>
                                    Quantity
                                </th>

                                <th>
                                    UoM
                                </th>

                            </tr>

                        </thead>

                        <tbody>
                `;


                for (
                    const item of warehouse.inventory
                ) {

                    html += `

                        <tr>

                            <td>
                                ${escapeHtml(
                                    item.materialID
                                )}
                            </td>

                            <td>
                                ${escapeHtml(
                                    item.materialName
                                )}
                            </td>

                            <td>
                                ${item.quantity}
                            </td>

                            <td>
                                ${escapeHtml(
                                    item.uom
                                )}
                            </td>

                        </tr>

                    `;
                }


                html += `

                        </tbody>

                    </table>

                `;
            }


            html += `

                </div>

            `;
        }


        content.innerHTML = html;

    }
    catch (error) {

        console.error(error);

        content.innerHTML = `

            <p>
                Could not connect to the server.
            </p>

        `;
    }
}

// ============================================================
// INVENTORY CHECK
// ============================================================

async function inventoryCheck() {

    const content =
        document.getElementById(
            "inventory-content"
        );

    content.innerHTML = `
        <p>
            Checking inventory...
        </p>
    `;


    try {

        const response =
            await fetch(
                "/api/inventory/check"
            );


        const data =
            await response.json();


        if (!response.ok) {

            content.innerHTML = `

                <div class="empty-message">

                    Inventory check failed.

                    <br><br>

                    ${escapeHtml(
                        data.message || ""
                    )}

                </div>

            `;

            return;
        }


        // ----------------------------------------------------
        // Inventory OK
        // ----------------------------------------------------

        if (data.valid) {

            content.innerHTML = `

                <div class="search-result">

                    <h3>
                        ✓ Inventory Check Passed
                    </h3>

                    <p>
                        No inconsistencies were found.
                    </p>

                    <p>
                        Warehouses checked:
                        <strong>
                            ${data.warehousesChecked}
                        </strong>
                    </p>

                    <p>
                        Inventory entries checked:
                        <strong>
                            ${data.entriesChecked}
                        </strong>
                    </p>

                </div>

            `;

            return;
        }


        // ----------------------------------------------------
        // Problems found
        // ----------------------------------------------------

        let html = `

            <div class="search-result">

                <h3>
                    Inventory Check
                </h3>

                <p>
                    The following inconsistencies were found:
                </p>

                <ul>
        `;


        for (
            const problem of data.problems
        ) {

            html += `

                <li>
                    ${escapeHtml(problem)}
                </li>

            `;
        }


        html += `

                </ul>

            </div>

        `;


        content.innerHTML = html;

    }
    catch (error) {

        console.error(error);

        content.innerHTML = `

            <p>
                Could not connect to the server.
            </p>

        `;
    }
}

// ============================================================
// CREATE PRODUCT FORM
// ============================================================

function showCreateProduct() {

    const content =
        document.getElementById(
            "product-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Create Product
            </h2>


            <label>
                Product ID
            </label>

            <input
                type="text"
                id="product-id"
                placeholder="###-######"
                maxlength="10"
            >


            <label>
                Product Name
            </label>

            <input
                type="text"
                id="product-name"
                placeholder="Enter Product Name"
            >


            <label>
                Description
            </label>

            <textarea
                id="product-description"
                rows="4"
                placeholder="Enter Product Description"></textarea>


            <h3>
                Bill of Materials
            </h3>


            <div id="bom-items">
            </div>


            <button
                type="button"
                onclick="addBOMItemRow()">

                + Add Component

            </button>


            <div class="form-actions">

                <button
                    onclick="createProduct()">

                    Create Product

                </button>

            </div>


            <div id="product-message">
            </div>

        </div>
    `;
}

// ============================================================
// ADD BOM ITEM
// ============================================================

function addBOMItemRow() {

    const container =
        document.getElementById(
            "bom-items"
        );


    const row =
        document.createElement(
            "div"
        );

    row.className =
        "bom-row";


    row.innerHTML = `

        <input
            type="text"
            class="bom-material-id"
            placeholder="Material ID"
            maxlength="10"
        >


        <input
            type="number"
            class="bom-quantity"
            min="1"
            placeholder="Quantity"
        >


        <button
            type="button"
            onclick="this.parentElement.remove()">

            Remove

        </button>

    `;


    container.appendChild(row);
}

// ============================================================
// CREATE PRODUCT
// ============================================================

async function createProduct() {

    const id =
        document.getElementById(
            "product-id"
        ).value.trim();

    const name =
        document.getElementById(
            "product-name"
        ).value.trim();

    const description =
        document.getElementById(
            "product-description"
        ).value.trim();


    const message =
        document.getElementById(
            "product-message"
        );


    const productIDPattern =
        /^[0-9]{3}-[0-9]{6}$/;


    // --------------------------------------------------------
    // Validate Product ID
    // --------------------------------------------------------

    if (!productIDPattern.test(id)) {

        message.textContent =
            "Invalid Product ID. Expected format ###-######.";

        return;
    }


    // --------------------------------------------------------
    // Validate required fields
    // --------------------------------------------------------

    if (!name) {

        message.textContent =
            "Product Name is required.";

        return;
    }


    // --------------------------------------------------------
    // Read BOM rows
    // --------------------------------------------------------

    const rows =
        document.querySelectorAll(
            ".bom-row"
        );


    const bom = [];


    for (const row of rows) {

        const materialID =
            row.querySelector(
                ".bom-material-id"
            ).value.trim();


        const quantity =
            Number(
                row.querySelector(
                    ".bom-quantity"
                ).value
            );


        if (!/^([0-9]{3}-[0-9]{6}|[0-9]{6}-00)$/.test(materialID))
        {
            message.textContent =
                "Invalid Material ID in BOM.";

            return;
        }


        if (quantity <= 0)
        {
            message.textContent =
                "BOM quantity must be greater than zero.";

            return;
        }


        bom.push({

            materialID:
                materialID,

            quantity:
                quantity

        });
    }


    // --------------------------------------------------------
    // Create product object
    // --------------------------------------------------------

    const product = {

        id: id,

        name: name,

        description: description,

        bom: bom

    };


    // --------------------------------------------------------
    // Send to C++
    // --------------------------------------------------------

    try {

        const response =
            await fetch(
                "/api/products/create",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(product)
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Product created successfully.";

            clearInputFields([
                "product-id",
                "product-name",
                "product-description"
            ]);
        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// DISPLAY PRODUCTS
// ============================================================

async function showDisplayProducts() {

    const content =
        document.getElementById(
            "product-content"
        );


    content.innerHTML = `
        <p>
            Loading products...
        </p>
    `;


    try {

        const response =
            await fetch(
                "/api/products"
            );


        if (!response.ok) {

            const message =
                await response.text();

            content.innerHTML = `
                <p>
                    Error: ${escapeHtml(message)}
                </p>
            `;

            return;
        }


        const data =
            await response.json();


        if (
            !data.products ||
            data.products.length === 0
        ) {

            content.innerHTML = `
                <div class="empty-message">
                    No products available.
                </div>
            `;

            return;
        }


        let html = "";


        for (
            const product of data.products
        ) {

            html += `

                <div class="material-table-container">

                    <h2>
                        ${escapeHtml(product.id)}
                        -
                        ${escapeHtml(product.name)}
                    </h2>

                    <p>
                        ${escapeHtml(
                            product.description || ""
                        )}
                    </p>
            `;


            if (
                !product.bom ||
                product.bom.length === 0
            ) {

                html += `

                    <div class="empty-message">
                        No BOM components.
                    </div>

                `;

            }
            else {

                html += `

                    <table class="material-table">

                        <thead>

                            <tr>

                                <th>
                                    Material ID
                                </th>

                                <th>
                                    Quantity
                                </th>

                            </tr>

                        </thead>

                        <tbody>
                `;


                for (
                    const item of product.bom
                ) {

                    html += `

                        <tr>

                            <td>
                                ${escapeHtml(
                                    item.materialID
                                )}
                            </td>

                            <td>
                                ${item.quantity}
                            </td>

                        </tr>

                    `;
                }


                html += `

                        </tbody>

                    </table>

                `;
            }


            html += `
                </div>
            `;
        }


        content.innerHTML = html;

    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>
                Could not connect to the server.
            </p>
        `;
    }
}

// ============================================================
// DELETE PRODUCT FORM
// ============================================================

function showDeleteProduct() {

    const content =
        document.getElementById(
            "product-content"
        );


    content.innerHTML = `

        <div class="form-container">

            <h2>
                Delete Product
            </h2>


            <label>
                Product ID
            </label>

            <input
                type="text"
                id="delete-product-id"
                placeholder="###-######"
                maxlength="10"
            >


            <div class="form-actions">

                <button
                    onclick="deleteProduct()">

                    Delete Product

                </button>

            </div>


            <div id="delete-product-message">
            </div>

        </div>
    `;
}

// ============================================================
// DELETE PRODUCT
// ============================================================

async function deleteProduct() {

    const id =
        document.getElementById(
            "delete-product-id"
        ).value.trim();


    const message =
        document.getElementById(
            "delete-product-message"
        );


    const productIDPattern =
        /^[0-9]{3}-[0-9]{6}$/;


    if (!productIDPattern.test(id)) {

        message.textContent =
            "Invalid Product ID. Expected format ###-######.";

        return;
    }


    try {

        const response =
            await fetch(
                "/api/products/delete",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify({
                            id: id
                        })
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Product deleted successfully.";

            document.getElementById(
                "delete-product-id"
            ).value = "";

        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// SELL PRODUCT FORM
// ============================================================

function showSellProduct() {

    const content =
        document.getElementById(
            "product-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Sell Product
            </h2>


            <label>
                Warehouse ID
            </label>

            <input
                type="number"
                id="sell-warehouse-id"
                placeholder="Enter Warehouse ID"
            >


            <label>
                Product ID
            </label>

            <input
                type="text"
                id="sell-product-id"
                placeholder="###-######"
                maxlength="10"
            >


            <label>
                Quantity Sold
            </label>

            <input
                type="number"
                id="sell-quantity"
                min="1"
                placeholder="Enter Quantity"
            >


            <div class="form-actions">

                <button
                    onclick="sellProduct()">

                    Sell Product

                </button>

            </div>


            <div id="sell-product-message">
            </div>

        </div>
    `;
}

// ============================================================
// SELL PRODUCT
// ============================================================

async function sellProduct() {

    const warehouseID =
        document.getElementById(
            "sell-warehouse-id"
        ).value;

    const productID =
        document.getElementById(
            "sell-product-id"
        ).value.trim();

    const quantity =
        document.getElementById(
            "sell-quantity"
        ).value;


    const message =
        document.getElementById(
            "sell-product-message"
        );


    const productIDPattern =
        /^[0-9]{3}-[0-9]{6}$/;


    // --------------------------------------------------------
    // Validation
    // --------------------------------------------------------

    if (!warehouseID) {

        message.textContent =
            "Please enter a Warehouse ID.";

        return;
    }


    if (!productIDPattern.test(productID)) {

        message.textContent =
            "Invalid Product ID. Expected format ###-######.";

        return;
    }


    if (!quantity ||
        Number(quantity) <= 0) {

        message.textContent =
            "Quantity must be greater than zero.";

        return;
    }


    const sale = {

        warehouseID:
            Number(warehouseID),

        productID:
            productID,

        quantity:
            Number(quantity)

    };


    // --------------------------------------------------------
    // Send to C++
    // --------------------------------------------------------

    try {

        const response =
            await fetch(
                "/api/products/sell",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(sale)
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Product sold successfully.";

            document.getElementById(
                "sell-quantity"
            ).value = "";

        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// READ FILE AS BASE64
// ============================================================

function readFileAsBase64(file) {

    return new Promise(
        (resolve, reject) => {

            const reader =
                new FileReader();

            reader.onload = () => {

                resolve(
                    reader.result
                );
            };

            reader.onerror = () => {

                reject(
                    new Error(
                        "Could not read image file."
                    )
                );
            };

            reader.readAsDataURL(file);
        }
    );
}

// ============================================================
// CLEAR INPUT FIELDS
// ============================================================

function clearInputFields(ids) {

    ids.forEach(id => {

        const element =
            document.getElementById(id);

        if (element) {

            element.value = "";

        }
    });
}

// ============================================================
// SET DATA LOGGING
// ============================================================

async function setDataLogging() {

    const checkbox =
        document.getElementById(
            "log-data-operations"
        );

    const message =
        document.getElementById(
            "data-logging-message"
        );


    try {

        const response =
            await fetch(
                "/api/data/logging",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify({
                            enabled:
                                checkbox.checked
                        })
                }
            );


        const data =
            await response.json();


        if (!response.ok) {

            message.textContent =
                data.message ||
                "Could not update logging setting.";

            return;
        }


        message.textContent =
            data.message;

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// BACK TO DASHBOARD
// ============================================================

function closeModule() {

    const dashboard =
        document.querySelector(
            ".dashboard"
        );

    const moduleView =
        document.getElementById(
            "module-view"
        );


    moduleView.classList.add(
        "hidden"
    );

    dashboard.style.display =
        "grid";
}